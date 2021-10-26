#pragma once
#include <deque>
#include <glm/glm.hpp>
#include "Events.h"

namespace Input
{

	enum Side
	{
		leftSide,
		rightSide
	};

	enum Action
	{
		nothing = 0,
		left = 1, // 1 << 0
		right = 2, // 1 << 1
		down = 4, // 1 << 2
		up  = 8, // 1 << 3
		light = 16, // 1 << 4
		medium = 32, // 1 << 5
		strong = 64,// 1 << 6
		ultra = 128, // 1 << 7
	};

	typedef std::pair<Input::Action, float> InputTime;//deque will store current input with a time.  When the element is first added to the deque time will be initalized to INPUT_TIME_IN_QUEUE.  Decrement the time by the frameTimeDelta and once that time is <= 0 remove from queue;
}

class InputHandler
{
public:
	InputHandler() = default;

	void handleInputPressed(Events::KeyPressedEvent& e);
	void handleInputReleased(Events::KeyReleasedEvent& e);
	void updateInputQueue(float deltaTime);
	bool isSequenceInInputQueue(std::vector<uint8_t> inputs) const;
	void clearInputQueue();
public:
	glm::vec2 currentMovementInput_;
	uint8_t currentAttackInput_ = 0;

	Input::Side side_ = Input::Side::leftSide;

	std::deque<Input::InputTime> inputQueue_;//Using a deque since elements are only going to be removed from the front but still need to update every element's time every frame.  So its more efficient than a vector(constant insertion and deletion for first and last element) but more flexible than a queue(can iterate over each element)

private:
	const float INPUT_TIME_IN_QUEUE = 0.5f;//seconds
};
