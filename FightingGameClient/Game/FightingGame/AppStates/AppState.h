#pragma once
class AppState
{
public:
	virtual void enterState() = 0;
	virtual AppState* update(float deltaTime) = 0;
	virtual ~AppState() = default;

	static AppState* transitionAppState(AppState* newState)
	{
		newState->enterState();
		return newState;
	}
};

