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

	glm::vec3 Camera::interpolate(glm::vec3 cameraPosStart, glm::vec3 cameraPosStop, glm::vec3 cameraTargetStart, glm::vec3 cameraTargetStop, double elapsedTime, double totalTime)
	{
		float t = elapsedTime / totalTime;
		cameraPosition = (1 - t) * cameraPosStart + t * cameraPosStop;
		cameraTarget = (1 - t) * cameraTargetStart + t * cameraTargetStop;
		cameraDirection = glm::normalize(cameraTarget - cameraPosition);
		cameraRightDirection = glm::normalize(glm::cross(cameraDirection, glm::vec3(0, 1, 0)));
		return cameraPosition;
	}
    
	std::vector<glm::vec3> deCasteljau(std::vector<glm::vec3> points, int degree, float t) {
		float *pointsQ = new float[(degree + 1) * 3];
		float *pointsR = new float[(degree + 1) * 3];
		int Qwidth = 3;
		for (int j = 0; j <= degree; j++) {
			pointsQ[j*Qwidth + 0] = points[j].x;
			pointsQ[j*Qwidth + 1] = points[j].y;
			pointsQ[j*Qwidth + 2] = points[j].z;
			pointsR[j*Qwidth + 0] = points[j].x;
			pointsR[j*Qwidth + 1] = points[j].y;
			pointsR[j*Qwidth + 2] = points[j].z;
		}
		for (int k = 1; k <= degree; k++) {
			for (int j = 0; j <= degree - k; j++) {
				pointsQ[j*Qwidth + 0] = (1 - t) * pointsQ[j*Qwidth + 0] + t * pointsQ[(j + 1)*Qwidth + 0];
				pointsQ[j*Qwidth + 1] = (1 - t) * pointsQ[j*Qwidth + 1] + t * pointsQ[(j + 1)*Qwidth + 1];
				pointsQ[j*Qwidth + 2] = (1 - t) * pointsQ[j*Qwidth + 2] + t * pointsQ[(j + 1)*Qwidth + 2];
				pointsR[j*Qwidth + 0] = (1 - (t + 0.1)) * pointsR[j*Qwidth + 0] + (t + 0.1) * pointsR[(j + 1)*Qwidth + 0];
				pointsR[j*Qwidth + 1] = (1 - (t + 0.1)) * pointsR[j*Qwidth + 1] + (t + 0.1) * pointsR[(j + 1)*Qwidth + 1];
				pointsR[j*Qwidth + 2] = (1 - (t + 0.1)) * pointsR[j*Qwidth + 2] + (t + 0.1) * pointsR[(j + 1)*Qwidth + 2];
			}
		}
		std::vector<glm::vec3> result;
		glm::vec3 resultPos;
		glm::vec3 resultTarg;
		resultPos.x = pointsQ[0];
		resultPos.y = pointsQ[1];
		resultPos.z = pointsQ[2];
		resultTarg.x = pointsR[0];
		resultTarg.y = pointsR[1];
		resultTarg.z = pointsR[2];
		delete[] pointsQ;
		delete[] pointsR;
		result.push_back(resultPos);
		result.push_back(resultTarg);
		return result;
	}

	glm::vec3 Camera::interpolateBezier(std::vector<glm::vec3> points, double elapsedTime, double totalTime) {
		float t = elapsedTime / totalTime;
		std::vector<glm::vec3> result = deCasteljau(points, points.size() - 1, t);
		cameraPosition = result[0];
		cameraTarget = result[1];
		cameraDirection = glm::normalize(cameraTarget - cameraPosition);
		cameraRightDirection = glm::normalize(glm::cross(cameraDirection, glm::vec3(0, 1, 0)));
		return cameraPosition;
	}
}
