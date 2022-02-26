#include "AttackTypes.h"
#include "../Fighter/Fighter.h"

class MoveHandler 
{
public:
	typedef Fighter* FighterType;
	typedef MoveInfo* MoveType;

	MoveHandler();
	~MoveHandler() = default;

	void setMove(FighterType fighter, MoveType moveInfo);
	bool updateMove(FighterType fighter);//true if move is done
	void addNextMove(MoveType moveInfo);
private:
	void handleFrameTags(FighterType fighter);

private:
	//Fighter* fighter_;
	//Animator* animator_;
	MoveInfo* currentMove_;
	MoveInfo* cancelMoveInto_;
	MoveInfo* nextMove_;
	int currFrame_;
	int totalFrames_;
};
