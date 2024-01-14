#include "Player.h"

void Player::render()
{
	//render red filled quad
	SDL_Rect fillRect = { p.x, p.y, DOT_WIDTH, DOT_HEIGHT };
	SDL_SetRenderDrawColor( gRenderer, 0x00, 0x7F, 0x00, 0xFF );
	SDL_RenderFillRect( gRenderer, &fillRect );
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
	SDL_RenderDrawLine( gRenderer, p.x + DOT_WIDTH/2, p.y + DOT_HEIGHT/2, p.x + DOT_WIDTH/2 + d.x*(DOT_WIDTH), p.y + DOT_HEIGHT/2 + d.y*(DOT_HEIGHT));
}

void Player::move()
{
	if (eController == nullptr) return;

	vel.x = 0;
	vel.y = 0;

	bool* keysPressed = eController->getKeysPressed();

	if (keysPressed[SupportedKeys::A]) {
		angle -= 0.1;
		if (angle < 0)
		{
			angle += cfg.M_PI_X_2;
		}
		d.x = cos(angle) * MAX_VEL;
		d.y = sin(angle) * MAX_VEL;
	}
	else if (keysPressed[SupportedKeys::D]) {
		angle += 0.1;
		if (angle > cfg.M_PI_X_2)
		{
			angle -= cfg.M_PI_X_2;
		}
		d.x = cos(angle) * MAX_VEL;
		d.y = sin(angle) * MAX_VEL;
	}

	float dx;
	float dy;
	if (keysPressed[SupportedKeys::LEFT_ARROW]) {
		float leftAngle = angle - cfg.M_PI_HALF;
		if (leftAngle < 0)
		{
			leftAngle += cfg.M_PI_X_2;
		}
		dx = cos(leftAngle) * MAX_VEL;
		dy = sin(leftAngle) * MAX_VEL;
		vel.x = dx;
		vel.y = dy;
	}
	else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
		float rightAngle = angle + cfg.M_PI_HALF;
		if (rightAngle > cfg.M_PI_X_2)
		{
			rightAngle -= cfg.M_PI_X_2;
		}
		dx = cos(rightAngle) * MAX_VEL;
		dy = sin(rightAngle) * MAX_VEL;
		vel.x = dx;
		vel.y = dy;
	}

	if (keysPressed[SupportedKeys::UP_ARROW]) {
		if (keysPressed[SupportedKeys::LEFT_ARROW]) {
			float angle2 = angle - M_PI/4;
			if (angle2 < 0)
			{
				angle2 += cfg.M_PI_X_2;
			}
			vel.x = cos(angle2) * MAX_VEL;
			vel.y = sin(angle2) * MAX_VEL;
		}else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
			float angle2 = angle + M_PI/4;
			if (angle2 > cfg.M_PI_X_2)
			{
				angle2 -= cfg.M_PI_X_2;
			}
			vel.x = cos(angle2) * MAX_VEL;
			vel.y = sin(angle2) * MAX_VEL;
		}else {
			vel.x = d.x;
			vel.y = d.y;
		}
	}
	else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
		if (keysPressed[SupportedKeys::LEFT_ARROW]) {
			float angle2 = angle + M_PI + M_PI/4;
			if (angle2 > cfg.M_PI_X_2)
			{
				angle2 -= cfg.M_PI_X_2;
			}
			vel.x = cos(angle2) * MAX_VEL;
			vel.y = sin(angle2) * MAX_VEL;
		}else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
			float angle2 = angle - M_PI - M_PI/4;
			if (angle2 < 0)
			{
				angle2 += cfg.M_PI_X_2;
			}
			vel.x = cos(angle2) * MAX_VEL;
			vel.y = sin(angle2) * MAX_VEL;
		}else {
			vel.x = -d.x;
			vel.y = -d.y;
		}
	}

	p.x += vel.x;
	p.y += vel.y;

}
