#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

    struct CollisionObject {
        glm::vec3 minBounds;
        glm::vec3 maxBounds;
    };

    class Camera {

    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        void addCollisionObject(const CollisionObject& obj);
        glm::vec3 getCameraPosition() const {
            return cameraPosition;
        }

        glm::vec3 getCameraFrontDirection() const {
            return cameraFrontDirection;
        }

        glm::vec3 getCameraRightDirection() const {
            return cameraRightDirection;
        }

        glm::vec3 getCameraUpDirection() const {
            return cameraUpDirection;
        }

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        std::vector<CollisionObject> collisionObjects;
        bool checkCollision(const glm::vec3& newPosition);
    };
}

#endif /* Camera_hpp */
