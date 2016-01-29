#ifndef DWL__SIMULATION__PREVIEW_LOCOMOTION__H
#define DWL__SIMULATION__PREVIEW_LOCOMOTION__H

#include <dwl/simulation/FootSplinePatternGenerator.h>
#include <dwl/model/WholeBodyDynamics.h>
#include <dwl/model/FloatingBaseSystem.h>
#include <dwl/utils/DynamicLocomotion.h>


namespace dwl
{

namespace simulation
{

struct PreviewState
{
	PreviewState() : time(0.), head_pos(0.), head_vel(0.), head_acc(0.) {
		com_pos.setZero();
		com_vel.setZero();
		com_acc.setZero();
		cop.setZero();
	}

	double time;
	Eigen::Vector3d com_pos;
	Eigen::Vector3d com_vel;
	Eigen::Vector3d com_acc;
	double head_pos;
	double head_vel;
	double head_acc;
	Eigen::Vector3d cop;
	rbd::BodyPosition support_region;
	rbd::BodyVector foot_pos;
	rbd::BodyVector foot_vel;
	rbd::BodyVector foot_acc;
};

enum TypeOfPhases {STANCE, FLIGHT};
struct PreviewPhase
{
	PreviewPhase() : type(STANCE), feet(rbd::BodySelector()) {}
	PreviewPhase(enum TypeOfPhases _type,
				 rbd::BodySelector _feet = rbd::BodySelector()) :
					 type(_type), feet(_feet) {}

	TypeOfPhases type;
	rbd::BodySelector feet;
};

struct PreviewParams
{
	PreviewParams() : duration(0.), cop_shift(Eigen::Vector2d::Zero()),
			  length_shift(0.), head_acc(0.) {}
	PreviewParams(double _duration,
				  Eigen::Vector2d _cop_shift,
				  double _length_shift,
				  double _head_acc) : duration(_duration), cop_shift(_cop_shift),
						  length_shift(_length_shift), head_acc(_head_acc) {}

	double duration;
	Eigen::Vector2d cop_shift;
	double length_shift;
	double head_acc;
	PreviewPhase phase;
};

struct PreviewControl
{
	PreviewControl() : params(std::vector<PreviewParams>()),
			feet_shift(rbd::BodyVector()) {}
	PreviewControl(std::vector<PreviewParams> _params,
				   rbd::BodyVector _feet_shift) : params(_params),
						   feet_shift(_feet_shift) {}

	std::vector<PreviewParams> params;
	rbd::BodyVector feet_shift;
};

typedef std::vector<PreviewState> PreviewTrajectory;
typedef std::vector<PreviewPhase> PreviewSchedule;

struct SwingParams
{
	SwingParams() : duration(0.), feet_shift(rbd::BodyVector()) {}
	SwingParams(double _duration,
				rbd::BodyVector _feet_shift) : duration(_duration), feet_shift(_feet_shift) {}

	double duration;
	rbd::BodyVector feet_shift;
};

struct SLIPModel
{
	SLIPModel() : stiffness(0.) {}
	SLIPModel(double _stiffness) :  stiffness(_stiffness) {};

	double stiffness;
};

/**
 * @class PreviewLocomotion
 * @brief Describes a preview locomotion
 * This class describes a preview of the locomotion by the decomposition of the movements in two
 * phases: stance and flight phase. In every phase, a preview defines the dynamic response of the
 * system by using a closed-loop equation. These closed-loop equations describes the response of
 * the locomotion according to a predefined template model, i.e. low-dimensional model. For the
 * stance phase, we used a Spring Loaded Inverted Pendulum (SLIP) model to describes the horizontal
 * motion and spring-mass system for the vertical motion of the CoM. Additionally, we assume the
 * heading motion decouple from the horizontal and vertical one. On the other hand, the flight phase
 * is modeled using the projectile EoM. Finally, the preview locomotion is a consequence of a
 * sequence of stance and flight previews.
 */
class PreviewLocomotion
{
	public:
		/** @brief Constructor function */
		PreviewLocomotion();

		/** @brief Destructor function */
		~PreviewLocomotion();

		/**
		 * @brief Resets the system information from an URDF file
		 * @param std::string URDF filename
		 * @param std::string Semantic system description filename
		 */
		void resetFromURDFFile(std::string urdf_file,
							   std::string system_file = std::string());

		/**
		 * @brief Resets the system information from URDF model
		 * @param std::string URDF model
		 * @param std::string Semantic system description filename
		 */
		void resetFromURDFModel(std::string urdf_model,
								std::string system_file = std::string());

		/**
		 * @brief Sets the sample time of the preview trajectory
		 * @param double Sample time
		 */
		void setSampleTime(double sample_time);

		/**
		 * @brief Sets the Spring Loaded Inverted Pendulum (SLIP) model
		 * @param const SLIPModel& SLIP model
		 */
		void setModel(const SLIPModel& model);

		/**
		 * @brief Sets the step height for the swing trajectory generation
		 * @param double Step height
		 */
		void setStepHeight(double step_height);

		/**
		 * @brief Sets the force threshold used for detecting active contacts
		 * @param double Force threshold
		 */
		void setForceThreshold(double force_threshold);

		void multiPhasePreview(PreviewTrajectory& trajectory,
							   const PreviewState& state,
							   const PreviewControl& control);

		/**
		 * @brief Computes the preview of the stance-phase given the stance parameters
		 * The preview is computed according a Spring Linear Inverted Pendulum (SLIP) model, and by
		 * assuming that the Center of Pressure (CoP) and the pendulum length are linearly controlled
		 * @param PreviewTrajectory& Preview trajectory at the predefined sample time
		 * @param const PreviewState& Initial low-dimensional state
		 * @param const PreviewParams& Preview control parameters
		 */
		void stancePreview(PreviewTrajectory& trajectory,
						   const PreviewState& state,
						   const PreviewParams& params);

		/**
		 * @brief Computes the preview of the flight-phase given the duration of the phase
		 * The preview is computed according the projectile Equation of Motion (EoM), and assuming
		 * the non-changes in the angular momentum
		 * @param PreviewTrajectory& Preview trajectory at the predefined sample time
		 * @param const PreviewState& Initial low-dimensional state
		 * @param const PreviewParams& Preview control parameters
		 */
		void flightPreview(PreviewTrajectory& trajectory,
				   	   	   const PreviewState& state,
						   const PreviewParams& params);

		/**
		 * @brief Computes the swing trajectory of the contact
		 * @param PreviewTrajectory& Preview trajectory at the predefined sample time
		 * @param const PreviewState& Initial low-dimensional state
		 * @param const SwingParams& Preview control parameters
		 */
		void addSwingPattern(PreviewTrajectory& trajectory,
							 const PreviewState& state,
							 const SwingParams& params);

		/**
		 * @brief Gets the preview states for each phase once a multi-phase was
		 * computed, otherwise return zero transitions
		 * @param simulation::PreviewTrajectory& Preview transitions between phases
		 * @param const simulation::PreviewTrajectory& Generated preview trajectory
		 * @param const simulation::PreviewControl& Applied preview control
		 */
		void getPreviewTransitions(simulation::PreviewTrajectory& transitions,
				   	   	   	   	   const simulation::PreviewTrajectory& trajectory,
								   const simulation::PreviewControl& control);

		/** @brief Returns the floating-base system pointer */
		model::FloatingBaseSystem* getFloatingBaseSystem();

		/** @brief Returns the sample time */
		double getSampleTime();

		/**
		 * @brief Converts the preview state vector to whole-body state
		 * @param WholeBodyState& Whole-body state
		 * @param const PreviewStated& Preview state vector
		 */
		void toWholeBodyState(WholeBodyState& full_state,
							  const PreviewState& preview_state);

		/**
		 * @brief Converts the whole-body state to preview state vector
		 * @param PreviewStated& Preview state vector
		 * @param const WholeBodyState& Whole-body state
		 */
		void fromWholeBodyState(PreviewState& preview_state,
								const WholeBodyState& full_state);

		/**
		 * @brief Converts a preview trajectory to a whole-body trajectory
		 * @param WholeBodyTrajectory& Whole-body trajectory
		 * @param const PreviewTrajectory& Preview trajectory
		 */
		void toWholeBodyTrajectory(WholeBodyTrajectory& full_traj,
								   const PreviewTrajectory& preview_traj);


	private:
		/** @brief Foot pattern generator */
		simulation::FootSplinePatternGenerator foot_pattern_generator_;

		/** @brief Floating-base system information */
		model::FloatingBaseSystem system_;

		/** @brief Whole-body dynamics */
		model::WholeBodyDynamics dynamics_;

		/** @brief Sample time of the preview trajectory */
		double sample_time_;

		/** @brief Gravity acceleration magnitude */
		double gravity_;

		/** @brief Total mass of the system */
		double mass_;

		/** @brief SLIP model */
		SLIPModel slip_;

		/** @brief Step height for the swing generation */
		double step_height_;

		/** @brief Actual Center of Mass (CoM) of the system */
		Eigen::Vector3d actual_system_com_;

		/** @brief Force threshold */
		double force_threshold_;
};

} //@namespace simulation
} //@namespace dwl

#endif
