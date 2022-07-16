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

#include "paparazzi-mobility-model.h"
#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "position-allocator.h"

using namespace std;

namespace ns3 {

	NS_LOG_COMPONENT_DEFINE ("Paparazzi");
	NS_OBJECT_ENSURE_REGISTERED (PaparazziMobilityModel);

	double destX[10] = {125, 375, 625, 875, 1125, 1375, 1625, 1875, 2125, 2375};
	double destY[10] = {87.5, 337.5, 587.5, 837.5, 1087.5, 1337.5, 1587.5, 1837.5, 2087.5, 2337.5};
	int destCount = 0;

	TypeId PaparazziMobilityModel::GetTypeId (void){
			static TypeId tid = TypeId ("ns3::PaparazziMobilityModel")
				.SetParent<MobilityModel> ()
				.SetGroupName ("Mobility")
				.AddConstructor<PaparazziMobilityModel> ()
				.AddAttribute ("Bounds",
						"Bounds of the area to cruise.",
						BoxValue (Box (0, 1000.0, 0.0, 1000.0, 0.0, 1000.0)),
						MakeBoxAccessor (&PaparazziMobilityModel::m_bounds),
						MakeBoxChecker ())
				.AddAttribute ("Speed",
						"Velocity of nodes in the Paparazzi mobility model (m/s).",
						DoubleValue (15.0),
						MakeDoubleAccessor (&PaparazziMobilityModel::m_velocity),
						MakeDoubleChecker<double> ())
				.AddAttribute ("Radius",
						"A constant representing the radius of the arch (m).",
						DoubleValue (25.0),
						MakeDoubleAccessor (&PaparazziMobilityModel::m_radius),
						MakeDoubleChecker<double> ());
			return tid;
	}

	PaparazziMobilityModel::PaparazziMobilityModel (){
		m_uniformStartPosition = CreateObject<UniformRandomVariable>();
		m_mode=WAYPOINT;
		m_angle=0;
		m_isdone=false;
		m_first = true;
		m_firstTurn=true;
		m_startComplexMvt=true;
		m_isTurn=false;
		m_finishedPatrol = new uint32_t[100];
		m_counter = 0;
		m_event = Simulator::ScheduleNow (&PaparazziMobilityModel::Start, this);
		m_helper.Unpause ();
	}

	PaparazziMobilityModel::~PaparazziMobilityModel()
	{
		delete[] m_finishedPatrol;
	}

	void PaparazziMobilityModel::Start (void) {
		Time update;

		for (int i = 1; i <= 10; i++)
		{
			destX[i-1] = (m_bounds.xMax/10*i) - ((m_bounds.xMax/10)/2);
			destY[i-1] = (m_bounds.yMax/10*i) - ((m_bounds.xMax/10)/2) - ((m_bounds.yMax/10)/4);
		}

		if(m_first){
			m_first = false;
			m_current = DoGetPosition();
			//m_helper.SetPosition(Vector((int)m_bounds.xMin,(int)m_bounds.yMin,0));
			m_destination=RandomPosition(); //도착 지점은 random
			update=MoveWaypoint();
		}
		else {
			if (m_mode==WAYPOINT){
				//현재 위치가 도착지점이라면
				if((int)CalculateDistance(m_destination, m_helper.GetCurrentPosition())==0){
					m_isdone = false;
					m_angle = 0;
					m_mode=STAYAT;
				}
				else{
					update=MoveWaypoint();
				}
			}
			else if (m_mode==STAYAT){
				if(m_isdone){
					m_mode=WAYPOINT;
					m_destination=RandomPosition();
				}
				else {
					update=MoveStayAt();
				}
			}
			else if (m_mode==OVAL){
				if((int)CalculateDistance(m_origin, m_helper.GetCurrentPosition())==0){
					m_mode=EIGHT;
					m_origin=GetPosition();
					m_startComplexMvt=true;
					update=MoveEight();
				}
				else{
					update=MoveOval();
				}
			}
			else if (m_mode==EIGHT){
				update=MoveEight();
			}
		}
		m_current = DoGetPosition();
		m_helper.Update ();
		m_helper.Unpause ();
		DoWalk (update);
	}

	void 
	PaparazziMobilityModel::DoWalk (Time delayLeft)
	{
		m_helper.UpdateWithBounds (m_bounds);
		m_event = Simulator::Schedule (delayLeft, &PaparazziMobilityModel::Start, this);
		/*
		if (m_bounds.IsInside (m_next)){
			m_event = Simulator::Schedule (delayLeft, &PaparazziMobilityModel::Start, this);
		}
		else{
			//nextPosition = m_bounds.CalculateIntersection (position, speed);
			//Time delay = Seconds ((nextPosition.x - position.x) / speed.x);
			//m_event = Simulator::Schedule (delay, &PaparazziMobilityModel::Rebound, this,
			//                               delayLeft - delay);
		}*/
		NotifyCourseChange ();
	}

	Vector3D 
	PaparazziMobilityModel::RandomPosition(void) const
	{
		Vector3D posi(0,0,0);
		//posi.x = m_uniformStartPosition->GetValue(m_bounds.xMax-m_radius, m_bounds.xMin+m_radius);
		//posi.y = m_uniformStartPosition->GetValue(m_bounds.yMax-m_radius, m_bounds.yMin+m_radius);
		int tempX = (int)m_uniformStartPosition->GetValue(0, 10);
		int tempY = (int)m_uniformStartPosition->GetValue(0, 10);
		posi.x = destX[tempX];
		posi.y = destY[tempY];
		if (m_bounds.zMax>0)
			posi.z = m_uniformStartPosition->GetValue(m_bounds.zMax-m_radius, m_bounds.zMin+m_radius);
		destCount++;
		return posi;
	}

	bool 
	PaparazziMobilityModel::CircularMouvement(const int& endDegree, const int& angle)
	{
		m_current=m_helper.GetCurrentPosition();
		m_angle+=angle;
		double current_angle = M_PI*m_angle/180;
		Vector position (0,0,0);

		double arc = 0.0;

		if (m_bounds.xMax == 1000.0)
		{
			arc = 3.0;
		}
		else if (m_bounds.xMax == 1500.0)
		{
			arc = 4.5;
		}
		else if (m_bounds.xMax == 2000.0)
		{
			arc = 6.0;
		}
		position.y = m_current.y + (arc*std::sin(current_angle)); //todo
		position.x = m_current.x + (arc*std::cos(current_angle)); //todo
		m_helper.SetPosition(Vector (position.x, position.y, position.z));
		return (m_angle==endDegree)?true:false;
	}

	Time PaparazziMobilityModel::MoveWaypoint(){
		m_current=m_helper.GetCurrentPosition();
		double dx = (m_destination.x - m_current.x);
		double dy = (m_destination.y - m_current.y);
		double dz = (m_destination.z - m_current.z);
		double k = m_velocity / std::sqrt (dx*dx + dy*dy + dz*dz);
		m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
		return Seconds (CalculateDistance (m_destination, m_current) / m_velocity);
	}

	Time PaparazziMobilityModel::MoveStayAt(){
		//두 바퀴 돌면 종료
		m_isdone=CircularMouvement(720,ANGLE);

		if(m_isdone)
		{
			int tempX;
			int tempY;
			
			for(int i = 0; i < 10; i++)
			{
				if(m_destination.x == destX[i])
				{
					tempX = i;
				}
			}

			for(int i = 0; i < 10; i++)
			{
				if(m_destination.y == destY[i])
				{
					tempY = i;
				}
			}

			m_finishedPatrol[m_counter] = tempY*10+tempX;
			m_counter++;
		}

		return Seconds(1.9);
		//return Seconds(2.5);
	}

	uint32_t *
	PaparazziMobilityModel::GetFinishedPatrol(void)
	{
		return m_finishedPatrol;
	}

	int
	PaparazziMobilityModel::GetFinishedPatrolSize(void)
	{
		return m_counter;
	}

	Time PaparazziMobilityModel::MoveOval(){
		NS_LOG_UNCOND ("MoveOval");
		Time temps;
		if(m_startComplexMvt){
			m_destination=RandomPosition();
			temps=MoveWaypoint();
			m_isTurn=true;
			m_startComplexMvt=false;
			m_angle=ANGLE;
		}
		else {
			if (!m_isTurn){
				if(m_helper.GetCurrentPosition().x<=m_origin.x)
					m_destination.x=m_origin.x - 4;
				else
					m_destination.x=m_origin.x + 4;

				if(m_helper.GetCurrentPosition().y<=m_origin.y)
					m_destination.y=m_origin.y + 6;
				else
					m_destination.y=m_origin.y - 6;
				m_destination.z=m_origin.z;

				temps=MoveWaypoint();
				m_isTurn=true;
			}
			else {
				m_isTurn=!CircularMouvement(90,ANGLE);
				temps=Seconds(0.1);
			}
		}
		return temps;
	}

	Time PaparazziMobilityModel::MoveEight(){
		Time temps;
		if(m_startComplexMvt){
			m_destination=RandomPosition();
			temps=MoveWaypoint();
			m_isTurn=true;
			m_startComplexMvt=false;
			m_angle=ANGLE;
		}
		else {

			if(!m_isTurn){
				if(m_helper.GetCurrentPosition().x>m_origin.x)
					m_destination.x=m_origin.x - 4;
				else
					m_destination.x=m_origin.x + 4;

				if(m_helper.GetCurrentPosition().y>m_origin.y)
					m_destination.y=m_origin.y + 6;
				else
					m_destination.y=m_origin.y - 6;
				m_destination.z=m_origin.z;
				
				temps=MoveWaypoint();
				m_angle=270;
				m_isTurn=true;
			}
			else{
				if(m_firstTurn){
					m_first=false;
					m_isTurn=!CircularMouvement(90,ANGLE);
				}
				else{
					m_first=true;
					m_isTurn=!CircularMouvement(270,ANGLE); //TODO allow to choice circular mouvement
				}
				temps=Seconds(0.1);
			}
		}
		return temps;
	}

	void PaparazziMobilityModel::DoDispose (void){
		// chain up
		MobilityModel::DoDispose ();
	}

	Vector PaparazziMobilityModel::DoGetPosition (void) const{
		m_helper.UpdateWithBounds (m_bounds);
		return m_helper.GetCurrentPosition ();
	}

	void PaparazziMobilityModel::DoSetPosition (const Vector &position){
		NS_ASSERT (m_bounds.IsInside (position));
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&PaparazziMobilityModel::Start, this);
	}

	Vector PaparazziMobilityModel::DoGetVelocity (void) const{
		return m_helper.GetVelocity ();
	}


	int64_t PaparazziMobilityModel::DoAssignStreams (int64_t stream){
		m_uniformStartPosition->SetStream(stream);
		return 1;
	}
}
