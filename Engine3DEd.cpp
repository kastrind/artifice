#include "Engine3D.h"

void Engine3D::addModel(float editingWidth, float editingHeight, float editingDepth, float editingRotationX, float editingRotationY, float editingRotationZ, unsigned int editingCubemapNameIndex, unsigned int editingTextureNameIndex, bool editingIsSolid, shapetype editingShape, glm::vec3 position)
{
	if (editingShape == shapetype::CUBE)
	{
		cube cube(std::max(editingWidth, std::max(editingHeight, editingDepth)), editingRotationX, editingRotationY, editingRotationZ);
		cubeModel mdl(getTimeSinceEpoch(), cubePointsCnt, cubemapNames[editingCubemapNameIndex], position, cube, editingIsSolid);
		cubePointsCnt += mdl.modelMesh.tris.size() * 3;
		std::cout << "about to place model with id = " << mdl.id << std::endl;
		mtx.lock();
		ptrModelsToRender.push_back(std::make_shared<cubeModel>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	} else
	{
		model m;
		if (editingShape == shapetype::RECTANGLE)
		{
			rectangle rectangle(editingWidth, editingHeight, editingRotationX, editingRotationY, editingRotationZ);
			model mdl(getTimeSinceEpoch(), modelPointsCnt, textureNames[editingTextureNameIndex], position, rectangle, editingIsSolid);
			m = mdl;
		}else if (editingShape == shapetype::CUBOID)
		{
			cuboid cuboid(editingWidth, editingHeight, editingDepth, editingRotationX, editingRotationY, editingRotationZ);
			model mdl(getTimeSinceEpoch(), modelPointsCnt, textureNames[editingTextureNameIndex], position, cuboid, editingIsSolid);
			m = mdl;
		}
		modelPointsCnt += m.modelMesh.tris.size() * 3;
		std::cout << "about to place model with id = " << m.id << std::endl;
		mtx.lock();
		ptrModelsToRender.push_back(std::make_shared<model>(m));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	}
}

void Engine3D::addModel(model& mdl)
{
	mdl.id = getTimeSinceEpoch();
	std::cout << "about to place cube model with id = " << mdl.id << std::endl;
	if (mdl.modelMesh.shape == shapetype::CUBE)
	{
		mdl.sn = cubePointsCnt;
		cubePointsCnt += mdl.modelMesh.tris.size() * 3;
		mtx.lock();
		ptrModelsToRender.push_back(std::make_shared<cubeModel>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	} else
	{
		mdl.sn = modelPointsCnt;
		modelPointsCnt += mdl.modelMesh.tris.size() * 3;
		mtx.lock();
		ptrModelsToRender.push_back(std::make_shared<model>(mdl));
		editingModel = ptrModelsToRender.back();
		std::cout << "placed model has sn = " << editingModel->sn << std::endl;
		mtx.unlock();
	}
}

void Engine3D::removeModel(std::shared_ptr<model> m)
{
	mtx.lock();
	m->removeFlag = true;
	unsigned long i = 0;
	for (i; i < ptrModelsToRender.size(); i++)
	{
		if (ptrModelsToRender[i]->removeFlag) break;
	}
	unsigned long removeIndex = i;
	std::cout << "removing model with index = " << i << " and sn = " << ptrModelsToRender[i]->sn << std::endl;
	if (m->modelMesh.shape == shapetype::CUBE)
	{
		cubePointsCnt -= m->modelMesh.tris.size() * 3;
	}else 
	{
		modelPointsCnt -= m->modelMesh.tris.size() * 3;
	}
	for (i+=1; i < ptrModelsToRender.size(); i++)
	{
		if ((m->modelMesh.shape == shapetype::CUBE && ptrModelsToRender[i]->modelMesh.shape == shapetype::CUBE) ||
			(m->modelMesh.shape != shapetype::CUBE && ptrModelsToRender[i]->modelMesh.shape != shapetype::CUBE))
		{
			ptrModelsToRender[i]->sn -= m->modelMesh.tris.size() * 3;
		}
	}
	ptrModelsToRender[removeIndex].reset();
	ptrModelsToRender.erase(ptrModelsToRender.begin() + removeIndex);
	m.reset();
	modelsInFocus.clear();
	finalModelsToRender.clear();
	finalCubeModelsToRender.clear();
	mtx.unlock();
}

void Engine3D::edit(float elapsedTime)
{
	if (userMode != UserMode::EDITOR) return;

	if (updateVerticesFlag) return;

	if (eventController != nullptr)
	{
		//bool* keysPressed = eventController->getKeysPressed();
		bool isEdited = false;

		if (prevKeysPressed[SupportedKeys::L] && !keysPressed[SupportedKeys::L]) {
			isLightingEditingModeEnabled = !isLightingEditingModeEnabled;
			if (isLightingEditingModeEnabled) std::cout << "Lighting editing mode enabled" << std::endl;
			else std::cout << "Lighting editing mode disabled" << std::endl;
		}

		if (isLightingEditingModeEnabled) {
			// pressing LCTRL + mouse wheel up/down cycles through lighting edit options
			if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollDown(keysPressed, prevKeysPressed)) {
				if (--lightingEditOptionIndex > lightingEditOptions.size() - 1) lightingEditOptionIndex = lightingEditOptions.size() - 1;
				std::cout << "editing: " << lightingEditOptions[lightingEditOptionIndex] << std::endl;

			} else if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollUp(keysPressed, prevKeysPressed)) {
				if (++lightingEditOptionIndex > lightingEditOptions.size() - 1) lightingEditOptionIndex = 0;
				std::cout << "editing: " << lightingEditOptions[lightingEditOptionIndex] << std::endl;
			}
			return;
		}

		// pressing LCTRL + mouse wheel up/down cycles through edit options
		if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			if (--editOptionIndex > editOptions.size() - 1) editOptionIndex = editOptions.size() - 1;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		} else if (keysPressed[SupportedKeys::LEFT_CTRL] && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			if (++editOptionIndex > editOptions.size() - 1) editOptionIndex = 0;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		// increases/decreases collation height
		} else if (editOptions[editOptionIndex] == "collationHeight" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			collationHeight = std::max(--collationHeight, (unsigned int)1);
			std::cout << "collation height: " << collationHeight << std::endl;
		} else if (editOptions[editOptionIndex] == "collationHeight" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			collationHeight++;
			std::cout << "collation height: " << collationHeight << std::endl;

		// increases/decreases collation width
		} else if (editOptions[editOptionIndex] == "collationWidth" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			collationWidth = std::max(--collationWidth, (unsigned int)1);
			std::cout << "collation width: " << collationWidth << std::endl;
		} else if (editOptions[editOptionIndex] == "collationWidth" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			collationWidth++;
			std::cout << "collation width: " << collationWidth << std::endl;

		// cycles through shapes
		} else if (editOptions[editOptionIndex] == "shape" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			if (++edShapeInt > shapetype::CUBE) edShapeInt = 0;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;
		} else if (editOptions[editOptionIndex] == "shape" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			if (--edShapeInt > shapetype::CUBE) edShapeInt = shapetype::CUBE;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;

		// increases/decreases width
		} else if (editOptions[editOptionIndex] == "width" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingWidth = std::max(editingWidth - 0.1f, 0.1f);
			std::cout << "width: " << editingWidth << std::endl;
			isEdited = true;
		} else if (editOptions[editOptionIndex] == "width" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingWidth += 0.1f;
			std::cout << "width: " << editingWidth << std::endl;
			isEdited = true;

		// increases/decreases height
		} else if (editOptions[editOptionIndex] == "height" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingHeight = std::max(editingHeight - 0.1f, 0.1f);
			std::cout << "height: " << editingHeight << std::endl;
			isEdited = true;
		} else if (editOptions[editOptionIndex] == "height" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingHeight += 0.1f;
			std::cout << "height: " << editingHeight << std::endl;
			isEdited = true;

		// increases/decreases depth
		} else if (editOptions[editOptionIndex] == "depth" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingDepth = std::max(editingDepth - 0.1f, 0.1f);
			std::cout << "depth: " << editingDepth << std::endl;
			isEdited = true;
		} else if (editOptions[editOptionIndex] == "depth" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingDepth += 0.1f;
			std::cout << "depth: " << editingDepth << std::endl;
			isEdited = true;

		// increases/decreases X rotation
		} else if (editOptions[editOptionIndex] == "rotationX" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingRotationX = std::max(editingRotationX - 0.1f, -cfg.M_PI_HALF);
			if (std::abs(editingRotationX) > 0 && std::abs(editingRotationX) < 0.1f) { editingRotationX = 0.0f; }
			std::cout << "rotationX: " << editingRotationX << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationX" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingRotationX = std::min(editingRotationX + 0.1f, cfg.M_PI_HALF);
			if (std::abs(editingRotationX) > 0 && std::abs(editingRotationX) < 0.1f) { editingRotationX = 0.0f; }
			std::cout << "rotationX: " << editingRotationX << std::endl;

		// increases/decreases Y rotation
		} else if (editOptions[editOptionIndex] == "rotationY" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingRotationY = std::max(editingRotationY - 0.1f, -cfg.M_PI_HALF);
			if (std::abs(editingRotationY) > 0 && std::abs(editingRotationY) < 0.1f) { editingRotationY = 0.0f; }
			std::cout << "rotationY: " << editingRotationY << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationY" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingRotationY = std::min(editingRotationY + 0.1f, cfg.M_PI_HALF);
			if (std::abs(editingRotationY) > 0 && std::abs(editingRotationY) < 0.1f) { editingRotationY = 0.0f; }
			std::cout << "rotationY: " << editingRotationY << std::endl;

		// increases/decreases Z rotation
		} else if (editOptions[editOptionIndex] == "rotationZ" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingRotationZ = std::max(editingRotationZ - 0.1f, -cfg.M_PI_HALF);
			if (std::abs(editingRotationZ) > 0 && std::abs(editingRotationZ) < 0.1f) { editingRotationZ = 0.0f; }
			std::cout << "rotationZ: " << editingRotationZ << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationZ" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingRotationZ = std::min(editingRotationZ + 0.1f, cfg.M_PI_HALF);
			if (std::abs(editingRotationZ) > 0 && std::abs(editingRotationZ) < 0.1f) { editingRotationZ = 0.0f; }
			std::cout << "rotationZ: " << editingRotationZ << std::endl;

		// cycles through textures
		} else if (editOptions[editOptionIndex] == "texture" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			if (editingShape == shapetype::CUBE)
			{
				if (++editingCubemapNameIndex > cubemapNames.size() - 1) editingCubemapNameIndex = 0;
				std::cout << "cubemap: " << cubemapNames[editingCubemapNameIndex] << std::endl;
			}else
			{
				if (++editingTextureNameIndex > textureNames.size() - 1) editingTextureNameIndex = 0;
				std::cout << "texture: " << textureNames[editingTextureNameIndex] << std::endl;
			}
		} else if (editOptions[editOptionIndex] == "texture" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			if (editingShape == shapetype::CUBE)
			{
				if (--editingCubemapNameIndex > cubemapNames.size() - 1) editingCubemapNameIndex = cubemapNames.size() - 1;
				std::cout << "cubemap: " << cubemapNames[editingCubemapNameIndex] << std::endl;
			}else
			{
				if (--editingTextureNameIndex > textureNames.size() - 1) editingTextureNameIndex = textureNames.size() - 1;
				std::cout << "texture: " << textureNames[editingTextureNameIndex] << std::endl;
			}

		// toggles isSolid
		} else if (editOptions[editOptionIndex] == "isSolid" && eventController->scrollDown(keysPressed, prevKeysPressed)) {
			editingIsSolid = !editingIsSolid;
			std::cout << "isSolid: " << editingIsSolid << std::endl;
		} else if (editOptions[editOptionIndex] == "isSolid" && eventController->scrollUp(keysPressed, prevKeysPressed)) {
			editingIsSolid = !editingIsSolid;
			std::cout << "isSolid: " << editingIsSolid << std::endl;

		} else if (keysPressed[SupportedKeys::C] && keysPressed[SupportedKeys::LEFT_CTRL]) {
			mtx.lock();
			auto modelInFocus = *(modelsInFocus.begin());
			copyingModel = modelInFocus;
			std::cout << "copying model: " << copyingModel->id << std::endl;
			mtx.unlock();

		} else if (prevKeysPressed[SupportedKeys::S] && keysPressed[SupportedKeys::S] == false && keysPressed[SupportedKeys::LEFT_CTRL]) {
			mtx.lock();
			std::cout << "SAVING!" << std::endl;
			if (level) {
				level->light = light;
				level->pointLight = pointLight;
				level->modelPointsCnt = modelPointsCnt;
				level->cubePointsCnt = cubePointsCnt;
				level->models = ptrModelsToRender;
				level->playerPosition = getPersonPos();
				level->save();
			}
			mtx.unlock();
		}

		if (editingModel == nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]) {
			glm::vec3 position = personPos + (editingDepth + originalCollidingDistanceH) * personFront;
			if (copyingModel == nullptr)
			{
				addModel(editingWidth, editingHeight, editingDepth, editingRotationX, editingRotationY, editingRotationZ, editingCubemapNameIndex, editingTextureNameIndex, editingIsSolid, editingShape, position);
			}else
			{
				model m = *(copyingModel);
				m.position = position;
				addModel(m);
				copyingModel.reset();
			}
			personSpeedFactor /= 100;
			isEdited = true;
		}
		
		// there is a spawned model about to be placed
		if (editingModel != nullptr) {
			//real-time update of transformation, texture and isSolid
			editingModel->position = personPos + (editingDepth + originalCollidingDistanceH) * personFront;
			editingModel->rotate(editingRotationX, editingRotationY, editingRotationZ);
			editingModel->scale(editingWidth, editingHeight, editingDepth);
			editingModel->texture = editingModel->modelMesh.shape == shapetype::CUBE ? cubemapNames[editingCubemapNameIndex] : textureNames[editingTextureNameIndex];
			editingModel->isSolid = editingIsSolid;
		}

		// releasing left mouse click places a new model
		if (editingModel != nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]==false) {
			std::cout << "starting model placement" << std::endl;
			personSpeedFactor *= 100;
			axis heightAlongAxis = axis::Y;
			axis widthAlongAxis = axis::X;
			short dirX = personFront.x / std::abs(personFront.x);
			short dirY = personFront.y / std::abs(personFront.y);
			short dirZ = personFront.z / std::abs(personFront.z);

			glm::vec3 pos = glm::vec3(0, 0, 0);
			if (std::abs(personFront.z) > std::abs(personFront.x)*1.2f && std::abs(personFront.z) > std::abs(personFront.y)*1.2f)
			{
				heightAlongAxis = axis::Y;
				widthAlongAxis = axis::Z;
				pos.z = dirZ;
			} else if (std::abs(personFront.x) > std::abs(personFront.y)*1.2f && std::abs(personFront.x) > std::abs(personFront.z)*1.2f)
			{
				heightAlongAxis = axis::Y;
				widthAlongAxis = axis::X;
				pos.x = dirX;
			} else if (std::abs(personFront.y) > std::abs(personFront.x)*1.2f && std::abs(personFront.y) > std::abs(personFront.z)*1.2f)
			{
				if (std::abs(personFront.x) > std::abs(personFront.z)) {
					heightAlongAxis = axis::X;
					widthAlongAxis = axis::Z;
					pos.z = dirZ;
				} else {
					heightAlongAxis = axis::Z;
					widthAlongAxis = axis::X;
					pos.x = dirX;
				}
			}

			model m = *editingModel;
			//position the model or snap it to the closest in focus
			m.position = personPos + (editingDepth + originalCollidingDistanceH) * personFront;
			if (!modelsInFocus.empty()) {
				auto modelInFocus = *(modelsInFocus.begin());
				//std::cout << "model in focus id: " << modelInFocus->id << ", editing model id: " << editingModel->id << ", distance: " << modelInFocus->distance << ", editingDepth: " << editingDepth << ", originalCollidingDistanceH: " << originalCollidingDistanceH << std::endl;
				if (modelInFocus->id != editingModel->id && modelInFocus->distance < editingDepth + originalCollidingDistanceH) {
					std::cout << "snapping to model in focus!" << std::endl;
					m.snapTo(getCameraFront(), modelInFocus);
				}
			}
			removeModel(ptrModelsToRender.back());
			addModel(m);

			//glm::vec3 initialPos = m.position;
			glm::vec3 initialPos = editingModel->position;
			std::cout << "dir X Y Z : " << dirX << ", " << dirY << ", " << dirZ << std::endl;
			//mtx.lock();
			std::cout << "models size before: " << ptrModelsToRender.size() << std::endl;
			for (unsigned int i = 0; i < collationWidth; i++) {
				unsigned int j = i > 0 ? 0 : 1;
				for (j; j < collationHeight; j++) {
					std::cout << "into collation height loop!" << std::endl;
					if (heightAlongAxis == axis::Y) m.position.y += editingHeight;
					else if (heightAlongAxis == axis::X) m.position.x += dirX * editingWidth;
					else if (heightAlongAxis == axis::Z) m.position.z += dirZ *  editingDepth;
					addModel(m);
				}
				if (widthAlongAxis == axis::Z) m.position.z += dirZ * editingDepth;
				else if (widthAlongAxis == axis::X) m.position.x += dirX * editingWidth;
				else if (widthAlongAxis == axis::Y) m.position.y += editingHeight;
				if (heightAlongAxis == axis::Y) m.position.y = initialPos.y - editingHeight;
				else if (heightAlongAxis == axis::X) m.position.x = initialPos.x - dirX * editingWidth;
				else if (heightAlongAxis == axis::Z) m.position.z = initialPos.z - dirZ * editingDepth;
			}
			//mtx.unlock();
			std::cout << "models size after: " << ptrModelsToRender.size() << std::endl;
			editingModel = nullptr;
			isEdited = true;
			std::cout << "ended model placement" << std::endl;

		// right mouse click deletes the model currently in focus
		} else if (deletingModel == nullptr && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] && modelsInFocus.size() > 0) {
			mtx.lock();
			auto modelInFocus = *(modelsInFocus.begin());
			deletingModel = modelInFocus;
			mtx.unlock();

		} else if (deletingModel != nullptr && deletingModel->removeFlag==false && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK]==false) {
			removeModel(deletingModel);
			deletingModel.reset();
			isEdited = true;
		}
		updateVerticesFlag = isEdited;
	}

}

int64_t Engine3D::getTimeSinceEpoch()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string Engine3D::shapeTypeToString(shapetype s)
{
	switch (s) {
		case shapetype::RECTANGLE:
			return "rectangle";
		case shapetype::CUBOID:
			return "cuboid";
		case shapetype::CUBE:
			return "cube";
		default:
			return "";
	}
}
