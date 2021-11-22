#include "pch.h"

#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

#include "core/input.h"
#include "core/log.h"
#include "core/window.h"
#include "buffer/fbo.h"
#include "buffer/ubo.h"
#include "components/all.h"
#include "scene/entity.h"
#include "scene/factory.h"
#include "scene/renderer.h"
#include "scene/scene.h"
#include "scene/ui.h"
#include "utils/filesystem.h"

using namespace core;
using namespace buffer;
using namespace components;

namespace scene {

    Scene* Renderer::last_scene = nullptr;
    Scene* Renderer::curr_scene = nullptr;

    std::queue<entt::entity> Renderer::render_queue {};

    static std::unique_ptr<buffer::VAO> dummy_vao;
    static bool depth_prepass = false;

    void Renderer::MSAA(bool on) {
        // the built-in MSAA only works on the default framebuffer (without multi-pass)
        static GLint buffers = 0, samples = 0, max_samples = 0;
        if (samples == 0) {
            glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
            glGetIntegerv(GL_SAMPLES, &samples);
            glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
            CORE_ASERT(buffers > 0, "MSAA buffers are not available! Check your window context...");
            CORE_ASERT(samples == 4, "Invalid MSAA buffer size! 4 samples per pixel is not available...");
        }

        if (on) {
            glEnable(GL_MULTISAMPLE);
        }
        else {
            glDisable(GL_MULTISAMPLE);
        }
    }

    void Renderer::DepthPrepass(bool on) {
        depth_prepass = on;
    }

    void Renderer::DepthTest(bool on) {
        if (on) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LEQUAL);
            glDepthRange(0.0f, 1.0f);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }
    }

    void Renderer::StencilTest(bool on) {
        // todo
        if (on) {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xFF);
            glStencilFunc(GL_EQUAL, 1, 0xFF);  // discard fragments whose stencil values != 1
        }
        else {
            glDisable(GL_STENCIL_TEST);
        }
    }

    void Renderer::FaceCulling(bool on) {
        if (on) {
            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glCullFace(GL_BACK);
        }
        else {
            glDisable(GL_CULL_FACE);
        }
    }

    void Renderer::SeamlessCubemap(bool on) {
        if (on) {
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
        else {
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }

    void Renderer::SetFrontFace(bool ccw) {
        glFrontFace(ccw ? GL_CCW : GL_CW);
    }

    void Renderer::SetViewport(GLuint width, GLuint height) {
        glViewport(0, 0, width, height);
    }

    void Renderer::Attach(const std::string& title) {
        CORE_TRACE("Attaching scene \"{0}\" ......", title);

        Input::Clear();
        Input::ShowCursor();
        Window::Rename(title);
        Window::layer = Layer::ImGui;

        // the new scene must be fully loaded and initialized before being assigned to `curr_scene`,
        // otherwise `curr_scene` could be pointing to a scene that has dirty states and subsequent
        // operations could possibly throw an access violation that will crash the program.

        Scene* new_scene = factory::LoadScene(title);
        new_scene->Init();
        curr_scene = new_scene;
    }

    void Renderer::Detach() {
        CORE_TRACE("Detaching scene \"{0}\" ......", curr_scene->title);

        last_scene = curr_scene;
        curr_scene = nullptr;

        delete last_scene;  // every object in the scene will be destructed
        last_scene = nullptr;
    }

    void Renderer::Clear() {
        // for the default framebuffer, do not use black as the clear color, because we want
        // to clearly see what pixels are background, but black makes it hard to debug many
        // buffer textures. Deep blue is a nice color, think of it as Microsoft blue screen.
        // note: custom framebuffers should always use a black clear color to make sure the
        // render buffer is clean, we don't want any dirty values other than 0. However, you
        // should call the `Clear()` method on a framebuffer instead, this function is only
        // intended for clearing the default framebuffer.
        
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClearDepth(1.0f);
        glClearStencil(0);  // 8-bit integer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Renderer::Flush() {
        if constexpr (_freeglut) {
            glutSwapBuffers();
            glutPostRedisplay();
        }
        else {
            glfwSwapBuffers(Window::window_ptr);
            glfwPollEvents();
        }
    }

    void Renderer::Render() {
        auto& reg = curr_scene->registry;
        auto mesh_group = reg.group<Mesh>(entt::get<Transform, Tag, Material>);
        auto model_group = reg.group<Model>(entt::get<Transform, Tag, Material>);

        while (!render_queue.empty()) {
            auto& e = render_queue.front();

            // skip entities marked as null (a convenient mask to tell if an entity should be drawn)
            if (e == entt::null) {
                render_queue.pop();
                continue;
            }

            // entity is a native mesh
            if (mesh_group.contains(e)) {
                auto& transform = mesh_group.get<Transform>(e);
                auto& mesh      = mesh_group.get<Mesh>(e);
                auto& material  = mesh_group.get<Material>(e);
                auto tag        = mesh_group.get<Tag>(e).tag;

                material.SetUniform(0, depth_prepass);
                material.SetUniform(1, transform.transform);
                material.SetUniform(2, 0);  // primitive mesh does not need a material id

                if (material.Bind()) {
                    if (tag == ETag::Skybox) {
                        SetFrontFace(0);  // skybox has reversed winding order, we only draw the inner faces
                        mesh.Draw();
                        SetFrontFace(1);  // recover the global winding order
                        material.Unbind();
                    }
                    else {
                        mesh.Draw();
                        material.Unbind();
                    }
                }
            }

            // entity is an imported model
            else if (model_group.contains(e)) {
                auto& transform = model_group.get<Transform>(e);
                auto& model     = model_group.get<Model>(e);
                auto& material  = model_group.get<Material>(e);  // one material is reused for every mesh

                material.SetUniform(0, depth_prepass);
                material.SetUniform(1, transform.transform);

                for (auto& mesh : model.meshes) {
                    GLuint material_id = mesh.material_id;
                    auto& textures = model.textures[material_id];
                    auto& properties = model.properties[material_id];

                    // update material id for the current mesh
                    material.SetUniform(2, static_cast<int>(material_id));

                    // update textures for the current mesh if textures are available
                    if (size_t n = textures.size(); n > 0) {
                        for (size_t i = 0; i < n; i++) {
                            material.SetTexture(i, textures[i]);  // use array index as texture unit
                        }
                    }
                    // update properties only if no textures are available
                    else {
                        for (size_t i = 0; i < properties.size(); i++) {
                            auto visitor = [&i, &material](const auto& prop) {
                                // model properties use uniform locations starting from 100, so as not
                                // to conflict with user-defined and internally reserved uniforms
                                material.SetUniform(i + 100, prop);
                            };
                            std::visit(visitor, properties[i]);
                        }
                    }

                    // commit and push the updates to the shader (notice that the shader is really
                    // bound only once for the first mesh, there's no context switching afterwards
                    // because the material and shader is reused for every mesh, so this is cheap)
                    material.Bind();
                    mesh.Draw();
                }

                material.Unbind();  // we only need to unbind once since it's shared by all meshes
            }

            // a non-null entity must have either a mesh or a model component to be considered renderable
            else {
                CORE_ERROR("Entity {0} in the render list is non-renderable!", e);
                Clear();  // in this case just show a deep blue screen (UI stuff is separate)
            }

            render_queue.pop();
        }
    }

    void Renderer::DrawScene() {
        curr_scene->OnSceneRender();
    }

    void Renderer::DrawImGui() {
        bool switch_scene = false;
        std::string next_scene_title;

        if (ui::NewFrame(); true) {
            ui::DrawMenuBar(curr_scene->title, next_scene_title);
            ui::DrawStatusBar();

            if (!next_scene_title.empty()) {
                switch_scene = true;
                Clear();
                ui::DrawLoadingScreen();
            }
            else {
                if (Window::layer == Layer::ImGui) {
                    curr_scene->OnImGuiRender();
                }
                else {
                    ui::DrawCrosshair();
                }
            }

            ui::EndFrame();
        }

        Flush();

        if (switch_scene) {
            Detach();                  // blocking call
            Attach(next_scene_title);  // blocking call (could take 30 minutes if scene is huge)
        }
    }

    void Renderer::DrawQuad() {
        if (dummy_vao == nullptr) {
            dummy_vao = std::make_unique<VAO>();
        }

        // bufferless rendering in OpenGL:
        // https://trass3r.github.io/coding/2019/09/11/bufferless-rendering.html
        // https://stackoverflow.com/a/59739538/10677643
        dummy_vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

}
