#include "pch.h"

#include "core/input.h"
#include "core/clock.h"
#include "core/window.h"
#include "components/camera.h"
#include "components/transform.h"

using namespace core;

namespace components {

    Camera::Camera(Transform* t, View view)
        : fov(90.0f), near_clip(0.1f), far_clip(100.0f),
          T(t), view(view), move_speed(3.0f), rotate_speed(0.3f) {}

    Camera::Camera(Camera&& other) noexcept {
        *this = std::move(other);
    }

    Camera& Camera::operator=(Camera&& other) noexcept {
        if (this != &other) {
            std::swap(view, other.view);
            std::swap(T, other.T);
            std::swap(fov, other.fov);
            std::swap(near_clip, other.near_clip);
            std::swap(far_clip, other.far_clip);
            std::swap(move_speed, other.move_speed);
            std::swap(rotate_speed, other.rotate_speed);
        }

        return *this;
    }

    glm::mat4 Camera::GetViewMatrix() const {
        return glm::lookAt(T->position, T->position + T->forward, T->up);
    }

    glm::mat4 Camera::GetProjectionMatrix() const {
        return view == View::Orthographic
            ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_clip, far_clip)
            : glm::perspective(glm::radians(fov), Window::aspect_ratio, near_clip, far_clip);
    }

    void Camera::Update() {
        // rotation (based on our euler angles convention we need to invert the axis)
        float euler_y = T->rotation.y - Input::GetMouseAxis(Axis::Horizontal) * rotate_speed;
        float euler_x = T->rotation.x - Input::GetMouseAxis(Axis::Vertical) * rotate_speed;
        euler_x = glm::clamp(euler_x, -88.0f, 88.0f);  // clamp vertical rotation

        T->Rotate(glm::vec3(euler_x, euler_y, 0.0f));

        // camera zoom
        fov += Input::GetMouseZoom();
        fov = glm::clamp(fov, 1.0f, 90.0f);

        // translation
        float deltatime = Clock::delta_time;

        if (Input::IsKeyPressed('w')) {
            T->Translate(T->forward * (move_speed * deltatime));
        }

        if (Input::IsKeyPressed('s')) {
            T->Translate(-T->forward * (move_speed * deltatime));
        }

        if (Input::IsKeyPressed('a')) {
            T->Translate(-T->right * (move_speed * deltatime));
        }

        if (Input::IsKeyPressed('d')) {
            T->Translate(T->right * (move_speed * deltatime));
        }

        if (Input::IsKeyPressed('z')) {
            T->Translate(-T->up * (move_speed * deltatime));
        }

        if (Input::IsKeyPressed(VK_SPACE)) {
            T->Translate(T->up * (move_speed * deltatime));
        }
    }
}