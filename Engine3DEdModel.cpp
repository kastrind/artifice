#include "Engine3D.h"

bool Engine3D::placeCompoundModel()
{

	// pressing LCTRL + mouse wheel up/down cycles through edit options
	if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		if (--compoundModelEditOptionIndex > compoundModelEditOptions.size() - 1) compoundModelEditOptionIndex = compoundModelEditOptions.size() - 1;
		std::cout << "editing: " << compoundModelEditOptions[compoundModelEditOptionIndex] << std::endl;

	} else if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		if (++compoundModelEditOptionIndex > compoundModelEditOptions.size() - 1) compoundModelEditOptionIndex = 0;
		std::cout << "editing: " << compoundModelEditOptions[compoundModelEditOptionIndex] << std::endl;

	// cycles through compound models (important: must NOT be editing one already)
	} else if (compoundModelFileNames.size() > 0 && compoundModelEditOptions[compoundModelEditOptionIndex] == "model" && eventController->scrollUp(keysPressed, prevKeysPressed) && editingModel == nullptr) {
		if (++editingCompoundModelFileNameIndex > compoundModelFileNames.size() - 1) editingCompoundModelFileNameIndex = 0;
		std::cout << "model: " << compoundModelFileNames[editingCompoundModelFileNameIndex] << std::endl;
	} else if (compoundModelFileNames.size() > 0 && compoundModelEditOptions[compoundModelEditOptionIndex] == "model" && eventController->scrollDown(keysPressed, prevKeysPressed) && editingModel == nullptr) {
		if (--editingCompoundModelFileNameIndex > compoundModelFileNames.size() - 1) editingCompoundModelFileNameIndex = compoundModelFileNames.size() - 1;
		std::cout << "model: " << compoundModelFileNames[editingCompoundModelFileNameIndex] << std::endl;

	// decreases/increases scale
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "scale" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		editingScale = std::max(editingScale - 0.1f, 0.1f);
		std::cout << "scale: " << editingScale << std::endl;
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "scale" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		editingScale += 0.1f;
		std::cout << "scale: " << editingScale << std::endl;

	// decreases/increases X rotation
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationX/pitch" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		editingRotationX = std::max(editingRotationX - 0.1f, -cfg.M_PI_HALF);
		if (std::abs(editingRotationX) > 0 && std::abs(editingRotationX) < 0.1f) { editingRotationX = 0.0f; }
		std::cout << "rotationX/pitch: " << editingRotationX << std::endl;
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationX/pitch" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		editingRotationX = std::min(editingRotationX + 0.1f, cfg.M_PI_HALF);
		if (std::abs(editingRotationX) > 0 && std::abs(editingRotationX) < 0.1f) { editingRotationX = 0.0f; }
		std::cout << "rotationX/pitch: " << editingRotationX << std::endl;

	// decreases/increases Y rotation
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationY/yaw" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		editingRotationY = std::max(editingRotationY - 0.1f, -cfg.M_PI_HALF);
		if (std::abs(editingRotationY) > 0 && std::abs(editingRotationY) < 0.1f) { editingRotationY = 0.0f; }
		std::cout << "rotationY/yaw: " << editingRotationY << std::endl;
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationY/yaw" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		editingRotationY = std::min(editingRotationY + 0.1f, cfg.M_PI_HALF);
		if (std::abs(editingRotationY) > 0 && std::abs(editingRotationY) < 0.1f) { editingRotationY = 0.0f; }
		std::cout << "rotationY/yaw: " << editingRotationY << std::endl;

	// decreases/increases Z rotation
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationZ/roll" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		editingRotationZ = std::max(editingRotationZ - 0.1f, -cfg.M_PI_HALF);
		if (std::abs(editingRotationZ) > 0 && std::abs(editingRotationZ) < 0.1f) { editingRotationZ = 0.0f; }
		std::cout << "rotationZ/roll: " << editingRotationZ << std::endl;
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "rotationZ/roll" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		editingRotationZ = std::min(editingRotationZ + 0.1f, cfg.M_PI_HALF);
		if (std::abs(editingRotationZ) > 0 && std::abs(editingRotationZ) < 0.1f) { editingRotationZ = 0.0f; }
		std::cout << "rotationZ/roll: " << editingRotationZ << std::endl;
	}

	if (editingModel == nullptr && eventController->pressPlace(keysPressed, prevKeysPressed)) {
		glm::vec3 position = gridPersonPos + (editingDepth + originalCollidingDistanceH) * gridPersonFront;
		if (copyingModel == nullptr)
		{
			Transform transform(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
			std::shared_ptr<CompoundModel> editingCompoundModel = CompoundModel::create(cfg.COMPOUND_MODELS_PATH + cfg.PATH_SEP + compoundModelFileNames[editingCompoundModelFileNameIndex] + ".cmdl", &transform);
			editingCompoundModelPartsCount = editingCompoundModel->models.size();
			for (auto & mdl : editingCompoundModel->models) {
				addModel(*mdl);
			}
		}else
		{
			//TODO: ADD COPIED MODEL

		}
		//personSpeedFactor /= 100;
		slowDownBy(cfg.EDITOR_SLOWDOWN_PERCENTAGE);
		return true;
	}

	// if there is a spawned compound model about to be placed
	if (editingModel != nullptr) {
		glm::vec3 position = gridPersonPos + (editingDepth + originalCollidingDistanceH) * gridPersonFront;
		std::shared_ptr<model> editingCompoundModelHead = ptrModelsToRender[ptrModelsToRender.size() - editingCompoundModelPartsCount];

		editingCompoundModelHead->isRootModel = true;
		editingCompoundModelHead->position = position;
		editingCompoundModelHead->rotate(editingRotationX, editingRotationY, editingRotationZ);
		editingCompoundModelHead->modelMatrix = glm::translate(glm::mat4(1.0f), editingCompoundModelHead->position) * editingCompoundModelHead->rotationMatrix;
		editingCompoundModelHead->modelMatrix = glm::scale(editingCompoundModelHead->modelMatrix, glm::vec3(editingScale, editingScale, editingScale));
		editingCompoundModelHead->headModelScale = glm::vec3(editingScale);

		// go through the vector of pointers to models to render, from the back of the vector up to editingCompoundModelPartsCount number of models, to update their transform
		for (size_t i = 0; i < editingCompoundModelPartsCount - 1 && i < ptrModelsToRender.size(); ++i) {
			std::shared_ptr<model> mdl = ptrModelsToRender[ptrModelsToRender.size() - 1 - i];
			// real-time update of the compound model's transform
			mdl->modelMatrix = editingCompoundModelHead->modelMatrix * glm::translate(glm::mat4(1.0f), mdl->localOffset) * mdl->rotationMatrix;
		}
	}

	// if the user releases the place key to finish placing the compound model
	if (editingModel != nullptr && eventController->releasePlace(keysPressed, prevKeysPressed)) {

		// glm::vec3 position = gridPersonPos + (editingDepth + originalCollidingDistanceH) * gridPersonFront;
		// std::shared_ptr<model> editingCompoundModelHead = ptrModelsToRender[ptrModelsToRender.size() - editingCompoundModelPartsCount];

		// editingCompoundModelHead->position = position;
		// editingCompoundModelHead->rotate(editingRotationX, editingRotationY, editingRotationZ);
		// // scale this way to update the triangles
		// editingCompoundModelHead->scale(editingCompoundModelHead->getLocalWidth() * editingScale, editingCompoundModelHead->getLocalHeight() * editingScale, editingCompoundModelHead->getLocalDepth() * editingScale);
		// editingCompoundModelHead->headModelScale = glm::vec3(editingScale);
		// editingCompoundModelHead->isTouched = false;

		editingModel = nullptr;
		std::cout << "finished placing compound model." << std::endl;
		//personSpeedFactor *= 100;
		resetSpeed();

		return true;
	}

	return false;

}