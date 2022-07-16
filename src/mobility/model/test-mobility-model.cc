/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2017 University of Haute Alsace
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sebastien Bindel  <sebastien.bindel@scientist.fr>
 */

#include "test-mobility-model.h"
#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "position-allocator.h"

using namespace std;

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("TestMobilityModel");
	NS_OBJECT_ENSURE_REGISTERED (TestMobilityModel);

	TypeId TestMobilityModel::GetTypeId (void){
			static TypeId tid = TypeId ("ns3::TestMobilityModel")
				.SetParent<MobilityModel> ()
				.SetGroupName ("Mobility")
				.AddConstructor<TestMobilityModel> ()
				.AddAttribute ("Bounds",
						"Bounds of the area to cruise.",
						BoxValue (Box (0, 1000.0, 0.0, 1000.0, 0.0, 1000.0)),
						MakeBoxAccessor (&TestMobilityModel::m_bounds),
						MakeBoxChecker ())
				.AddAttribute ("Speed",
						"Velocity of nodes in the Paparazzi mobility model (m/s).",
						DoubleValue (15.0),
						MakeDoubleAccessor (&TestMobilityModel::m_velocity),
						MakeDoubleChecker<double> ());
			return tid;
		}

	TestMobilityModel::TestMobilityModel (){
		m_uniformStartPosition = CreateObject<UniformRandomVariable>();
		m_mode=WAYPOINT;
		m_first = true;
		m_counter = 0;
		m_event = Simulator::ScheduleNow (&TestMobilityModel::Start, this);
		m_helper.Unpause ();
		m_patrolNumber = 0;
	}

	void TestMobilityModel::Start (void) {
		Time update;

		if(m_first)
		{

			m_first = false;
			m_current = DoGetPosition();

			
			if(m_current.x < m_bounds.xMax && m_current.y > m_bounds.xMax)
			{
				m_counter = 1;
			}
			else if(m_current.x > m_bounds.xMax && m_current.y > m_bounds.xMax)
			{
				m_counter = 2;
			}
			else if(m_current.x > m_bounds.xMax && m_current.y < m_bounds.xMax)
			{
				m_counter = 3;
			}

			m_destination = RandomPosition();

			update=MoveWaypoint();

		}
		else 
		{

			if (m_mode==WAYPOINT){
				if((int)CalculateDistance(m_destination, m_helper.GetCurrentPosition())==0)
				{
					m_counter++;
					m_destination = RandomPosition();
				}
				else{

					update=MoveWaypoint();
				
				}
			}

		}
		m_current = DoGetPosition();
		m_helper.Update ();
		m_helper.Unpause ();
		DoWalk (update);
	}

	void 
	TestMobilityModel::DoWalk (Time delayLeft)
	{
		m_helper.UpdateWithBounds (m_bounds);
		m_event = Simulator::Schedule (delayLeft, &TestMobilityModel::Start, this);
		NotifyCourseChange ();
	}

	Vector3D 
	TestMobilityModel::RandomPosition(void) const
	{
		int a = ((m_bounds.xMax/10)*2) + ((m_bounds.xMax/10)/2);
		int b = m_bounds.xMax - a;
		double destX[4] = {625, 625, 1875, 1875};
		double destY[4] = {625, 1875, 1875, 625};

		destX[0] = a;
		destX[1] = a;
		destX[2] = b;
		destX[3] = b;

		destY[0] = a;
		destY[1] = b;
		destY[2] = b;
		destY[3] = a;

		Vector3D posi(0,0,0);

		int tempX = m_counter % 4;
		int tempY = m_counter % 4;

		posi.x = destX[tempX];
		posi.y = destY[tempY];

		return posi;
	}

	Time TestMobilityModel::MoveWaypoint(){
		m_current=m_helper.GetCurrentPosition();
		double dx = (m_destination.x - m_current.x);
		double dy = (m_destination.y - m_current.y);
		double dz = (m_destination.z - m_current.z);
		double k = m_velocity / std::sqrt (dx*dx + dy*dy + dz*dz);
		m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
		return Seconds (CalculateDistance (m_destination, m_current) / m_velocity);
	}

	void TestMobilityModel::DoDispose (void){
		// chain up
		MobilityModel::DoDispose ();
	}

	Vector TestMobilityModel::DoGetPosition (void) const{
		m_helper.UpdateWithBounds (m_bounds);
		return m_helper.GetCurrentPosition ();
	}

	void TestMobilityModel::DoSetPosition (const Vector &position){
		NS_ASSERT (m_bounds.IsInside (position));
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&TestMobilityModel::Start, this);
	}

	Vector TestMobilityModel::DoGetVelocity (void) const{
		return m_helper.GetVelocity ();
	}


	int64_t TestMobilityModel::DoAssignStreams (int64_t stream){
		m_uniformStartPosition->SetStream(stream);
		return 1;
	}
}
