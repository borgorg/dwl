#include <model/Constraint.h>


namespace dwl
{

namespace model
{

Constraint::Constraint() : constraint_dimension_(0)
{

}


Constraint::~Constraint()
{

}


void Constraint::setLastState(LocomotionState& last_state)
{
	last_state_ = last_state;
}


unsigned int Constraint::getConstraintDimension()
{
	constraint_dimension_ = defineConstraintDimension();
	return constraint_dimension_;
}


std::string Constraint::getName()
{
	return name_;
}

} //@namespace model
} //@namespace dwl
