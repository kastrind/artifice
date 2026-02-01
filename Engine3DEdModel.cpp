#include "Engine3D.h"

bool Engine3D::placeCompoundModel()
{
	readCompoundModelFileNames();

	// pressing LCTRL + mouse wheel up/down cycles through edit options
	if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollDown(keysPressed, prevKeysPressed)) {
		if (--compoundModelEditOptionIndex > compoundModelEditOptions.size() - 1) compoundModelEditOptionIndex = compoundModelEditOptions.size() - 1;
		std::cout << "editing: " << compoundModelEditOptions[compoundModelEditOptionIndex] << std::endl;

	} else if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		if (++compoundModelEditOptionIndex > compoundModelEditOptions.size() - 1) compoundModelEditOptionIndex = 0;
		std::cout << "editing: " << compoundModelEditOptions[compoundModelEditOptionIndex] << std::endl;

	// cycles through compound models
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "model" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
		if (++editingCompoundModelFileNameIndex > compoundModelFileNames.size() - 1) editingCompoundModelFileNameIndex = 0;
		std::cout << "model: " << compoundModelFileNames[editingCompoundModelFileNameIndex] << std::endl;
	} else if (compoundModelEditOptions[compoundModelEditOptionIndex] == "model" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
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

	if (editingModel == nullptr && eventController->place(keysPressed, prevKeysPressed)) {
		glm::vec3 position = personPos + (editingDepth + originalCollidingDistanceH) * personFront;
		if (copyingModel == nullptr)
		{	
			CompoundModel cModel;
			Transform transform(position, glm::vec3(editingRotationX, editingRotationY, editingRotationZ), glm::vec3(editingScale, editingScale, editingScale));
			cModel.load(cfg.ASSETS_PATH + cfg.PATH_SEP + "compoundModels" + cfg.PATH_SEP + compoundModelFileNames[editingCompoundModelFileNameIndex] + ".cmdl", &transform);
			for (auto & mdl : cModel.models) {
				addModel(*mdl);
				std::cout << "added model from compound model with id = " << mdl->id << std::endl;
			}
			editingModel = nullptr;
		}else
		{
			//TODO: ADD COPIED MODEL

		}
		return true;
	}

	return false;

}