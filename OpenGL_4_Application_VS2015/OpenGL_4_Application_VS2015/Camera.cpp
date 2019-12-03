//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	glm::vec3 Camera::getCameraDirection()
	{
		return cameraDirection;
	}

	glm::vec3 Camera::getCameraPosition()
	{
		return cameraPosition;
	}

	glm::vec3 Camera::getCameraRightDirection()
	{
		return cameraRightDirection;
	}

	
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;

			case MOVE_UP:
				cameraPosition += glm::cross(cameraRightDirection, cameraDirection) * speed;
				break;

			case MOVE_DOWN:
				cameraPosition -= glm::cross(cameraRightDirection, cameraDirection) * speed;
				break;
        }
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
		float dot = glm::dot(cameraDirection, glm::vec3(0, 1, 0));
		float angle = glm::degrees(glm::acos(dot));
		if (angle > 120 && pitch > 0)
			pitch = 0;
		if (angle < 60 && pitch < 0)
			pitch = 0;
		cameraDirection = glm::rotate(glm::mat4(1.0f), yaw, glm::cross(cameraRightDirection, cameraDirection))*glm::vec4(cameraDirection, 0.0f);
		cameraDirection = glm::rotate(glm::mat4(1.0f), -pitch, cameraRightDirection)*glm::vec4(cameraDirection, 0.0f);
		cameraRightDirection = glm::normalize(glm::cross(cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
		cameraTarget = cameraPosition + cameraDirection;
    }
    
}
