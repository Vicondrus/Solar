//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include <vector>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN};
    
    class Camera
    {
    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget);
        glm::mat4 getViewMatrix();
		glm::vec3 getCameraDirection();
		glm::vec3 getCameraPosition();
		glm::vec3 getCameraRightDirection();
        void move(MOVE_DIRECTION direction, float speed);
        void rotate(float pitch, float yaw);
		glm::vec3 interpolate(glm::vec3 cameraPosStart, glm::vec3 cameraPosStop, glm::vec3 cameraTargetStart, glm::vec3 cameraTargetStop, double elapsedTime, double totalTime);

		glm::vec3 interpolateBezier(std::vector<glm::vec3> points, double elapsedTime, double totalTime);

        
    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraDirection;
        glm::vec3 cameraRightDirection;
    };
    
}

#endif /* Camera_hpp */
