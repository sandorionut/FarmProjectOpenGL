#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        this->cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 newPosition = cameraPosition;

        if (direction == MOVE_FORWARD) {
            newPosition += speed * cameraFrontDirection;
        }
        if (direction == MOVE_BACKWARD) {
            newPosition -= speed * cameraFrontDirection;
        }
        if (direction == MOVE_LEFT) {
            newPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        }
        if (direction == MOVE_RIGHT) {
            newPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        }

        // Păstrăm coordonata y constantă
        newPosition.y = cameraPosition.y;
        // Verificăm coliziunile
        if (!checkCollision(newPosition)) {
            cameraPosition = newPosition;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);
    }

    void Camera::addCollisionObject(const CollisionObject& obj) {
        collisionObjects.push_back(obj);
    }

    bool Camera::checkCollision(const glm::vec3& newPosition) {
        for (const auto& obj : collisionObjects) {
            if (newPosition.x > obj.minBounds.x && newPosition.x < obj.maxBounds.x &&
                newPosition.y > obj.minBounds.y && newPosition.y < obj.maxBounds.y &&
                newPosition.z > obj.minBounds.z && newPosition.z < obj.maxBounds.z) {
                return true;
            }
        }
        return false;
    }
}
