#include "MoveHandler.h"

#define CHECK_TAG_SET(TAG, CHECK_IF_SET ) (TAG & CHECK_IF_SET) == CHECK_IF_SET

MoveHandler::MoveHandler(/*Fighter* fighter*/) :
	//fighter_(fighter),
	//animator_(&fighter->entity_->getComponent<Animator>()),
	currentMove_(nullptr),
	cancelMoveInto_(nullptr)
{}

void MoveHandler::setMove(MoveHandler::FighterType fighter, MoveHandler::MoveType moveInfo)
{
	currFrame_ = 0;
	MoveInfoComponent& moveInfoComponent = fighter->entity_->getComponent<MoveInfoComponent>();
	currentMove_ = moveInfoComponent.moveInfo_;
	currentMove_->hit_ = false;
	currentMove_->numHits_ = 0;
	cancelMoveInto_ = nullptr;
	nextMove_ = nullptr;
	fighter->entity_->getComponent<Animator>().setAnimation(moveInfo->animationName_);
	totalFrames_ = currentMove_->frameData_.size();
}

bool MoveHandler::updateMove(FighterType fighter)
{
	Animator& animator = fighter->entity_->getComponent<Animator>();
	AnimationClip* clip = animator.getAnimationClipByName(currentMove_->animationName_);
	//AnimationClip* clip = animator_->getAnimationClipByName(currentMove_->animationName_);
	float ratio = currFrame_ / (float)totalFrames_;
	float timeInSeconds = clip->durationInSeconds_ * ratio;
	animator.setAnimationTime(timeInSeconds);

	fighter->entity_->getComponent<Collider>().setColliders(currentMove_->frameData_[currFrame_].colliders_);

	handleFrameTags(fighter);

	currFrame_++;
	if (currFrame_ >= totalFrames_)//attack is finished
	{
		return true;
	}
	return false;
}

void MoveHandler::addNextMove(MoveType moveInfo)
{
	nextMove_ = moveInfo;
}

void MoveHandler::handleFrameTags(FighterType fighter)
{
	const FrameInfo& currentFrame = currentMove_->frameData_[currFrame_];
	const int frameTags = currentFrame.frameTags_;
	if (CHECK_TAG_SET(frameTags, FrameTags::CANCELABLE))
	{
		if (nextMove_)
		{
			for (int j = 0; j < currentFrame.cancelListIds_.size(); j++)
			{
				const std::vector<std::string>& cancelList = currentMove_->cancelMap_[currentFrame.cancelListIds_[j]];
				for (int k = 0; k < cancelList.size(); k++)
				{
					if (nextMove_->name_ == cancelList[k])
					{
						cancelMoveInto_ = nextMove_;
					}
				}
			}
		}
	}
	if (CHECK_TAG_SET(frameTags, FrameTags::EXECUTE_CANCEL))
	{
		if (cancelMoveInto_)
		{
			setMove(fighter, cancelMoveInto_);
		}
	}
	if (CHECK_TAG_SET(frameTags, FrameTags::SET_VELOCITY))
	{
		glm::vec4 newVelocityFighterSpace = { 0.0f, currentFrame.velocity_.y, currentFrame.velocity_.x , 0.0f};
		glm::mat fighterToWorld = fighter->entity_->getComponent<Transform>().calculateTransformNoScale();
		fighter->velocityWorldSpace_ = fighterToWorld * newVelocityFighterSpace;
	}
	if (CHECK_TAG_SET(frameTags, FrameTags::TRANSITION_TO_AERIAL))
	{
		fighter->applyGravity_ = true;
	}
}
