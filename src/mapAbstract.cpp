/**
 * mapAbstract.cpp
 *
 * \date 10/03/2010
 * \author jsola@laas.fr
 *
 *  \file mapAbstract.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#include "jmath/indirectArray.hpp"
#include <boost/shared_ptr.hpp>
#include "jmath/random.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/mapAbstract.hpp"
#include "rtslam/robotAbstract.hpp"
#include "rtslam/landmarkAbstract.hpp"

// \todo this needs to go out of here - when we'll have factories
#include "rtslam/observationPinHoleAnchoredHomogeneous.hpp"

namespace jafar {
	namespace rtslam {
		using namespace std;


		// Serializer function very long: and defined at the end of file
		// std::ostream& operator <<(std::ostream & s, jafar::rtslam::MapAbstract & map) {

		/**
		 * Constructor
		 */
		MapAbstract::MapAbstract(size_t _max_size) :
			max_size(_max_size), current_size(0), used_states(max_size) {
			used_states.clear();
			ekfInd_ptr_t filtPtr(new ExtendedKalmanFilterIndirect(_max_size));
			filterPtr = filtPtr;
		}
		MapAbstract::MapAbstract(const ekfInd_ptr_t & ekfPtr) :
			filterPtr(ekfPtr),
			max_size(ekfPtr->size()),
			current_size(0),
			used_states(ekfPtr->size())
		{
			used_states.clear();
		}

		jblas::vec & MapAbstract::x() {
			return filterPtr->x();
		}
		jblas::sym_mat & MapAbstract::P() {
			return filterPtr->P();
		}
		double & MapAbstract::x(size_t i) {
			return filterPtr->x(i);
		}
		double & MapAbstract::P(size_t i, size_t j) {
			return filterPtr->P(i, j);
		}

		jblas::ind_array MapAbstract::reserveStates(const std::size_t N) {
			if (unusedStates(N)) {
				jblas::ind_array res = jmath::ublasExtra::ia_pushfront(used_states, N);
				current_size += N;
				return res;
			}
			else {
				jblas::ind_array res(0);
				return res;
			}
		}

		void MapAbstract::liberateStates(const jblas::ind_array & _ia) {
			for (size_t i = 0; _ia.size(); i++)
				if (used_states(_ia(i)) == true) {
					used_states(_ia(i)) = false;
					current_size += 1;
				}
		}

		void MapAbstract::completeObservationsInGraph(const sensor_ptr_t & existingSenPtr, const landmark_ptr_t & lmkPtr) {
			for (RobotList::iterator robIter = robotList().begin(); robIter != robotList().end(); robIter++) {
				robot_ptr_t robPtr = *robIter;
				for (RobotAbstract::SensorList::iterator senIter = robPtr->sensorList().begin(); senIter != robPtr->sensorList().end(); senIter++) {
					sensor_ptr_t senPtr = *senIter;
					if (senPtr != existingSenPtr){
						observation_ptr_t obsPtr = newObservation(senPtr, lmkPtr);
					}
				}
			}
		}

		void MapAbstract::fillSeq() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				for (size_t j = 0; j < max_size; j++)
					P(i, j) = i + 100 * j;
			}
		}

		void MapAbstract::fillDiag() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				P(i, i) = 1;
			}
		}

		void MapAbstract::fillDiagSeq() {
			for (size_t i = 0; i < max_size; i++) {
				x(i) = i;
				P(i, i) = i;
			}
		}

		void MapAbstract::fillRndm() {
			randVector(x());
			randMatrix(P());
		}

		observation_ptr_t MapAbstract::newObservation(const sensor_ptr_t & senPtr, const landmark_ptr_t & lmkPtr) {
			// todo make obs creation dynamic with factories or switch or other.
			obs_ph_ahp_ptr_t obsPtr(new ObservationPinHoleAnchoredHomogeneousPoint(senPtr, lmkPtr));
			obsPtr->id() = lmkPtr->id();
//			obsPtr->linkToParentSensor(senPtr);
//			obsPtr->linkToParentLandmark(lmkPtr);
			obsPtr->linkToParentPinHole(senPtr);
			obsPtr->linkToParentAHP(lmkPtr);

			return obsPtr;
		}


		/**
		 * Serializer. Print all MAP data.
		 *
		 * It traverses the map tree in the following way:
		 * - robots
		 *   - sensors in robot
		 * - landmarks
		 *   - observations of landmark from each sensor
		 */
		std::ostream& operator <<(std::ostream & s, const jafar::rtslam::MapAbstract & map) {

			s << "\n% ROBOTS AND SENSORS \n%=========================" << endl;
			for (MapAbstract::RobotList::const_iterator robIter = map.robotList().begin(); robIter != map.robotList().end(); robIter++) {
				robot_ptr_t robPtr = *robIter;
				s << *robPtr << endl;
				for (RobotAbstract::SensorList::const_iterator senIter = robPtr->sensorList().begin(); senIter
				    != robPtr->sensorList().end(); senIter++) {
					sensor_ptr_t senPtr = *senIter;
					s << *senPtr << endl;
				}
			}
			s << "\n% LANDMARKS AND OBSERVATIONS \n%==========================" << endl;
			for (MapAbstract::LandmarkList::const_iterator lmkIter = map.landmarkList().begin(); lmkIter
			    != map.landmarkList().end(); lmkIter++) {
				landmark_ptr_t lmkPtr = *lmkIter;
				s << *lmkPtr << endl;
				for (LandmarkAbstract::ObservationList::iterator obsIter = lmkPtr->observationList().begin();
						obsIter != lmkPtr->observationList().end(); obsIter++) {
					observation_ptr_t obsPtr = *obsIter;
					s << *obsPtr << endl;
				}
			}
			return s;
		}

	}
}
