/** \file CCabin.cpp
 * \brief CCabin class source file.
 * \details Enthaelt die Implementierung der Klasse CCabin.
 * \author Reimund
 * \date 2016
 */

#include "CCabin.h"
#include"CCabinDoor.h"
#include<iostream>
#include<string>
using namespace std;

/*! \fn CCabin::CCabin()
 *  \brief Konstruktor. Belegt die Attribute mit Standardwerten (0, false)
 */
CCabin::CCabin(CCabinDoor::LAYOUT layout)
{
	m_height = 0.0;
	m_isDriving = false;
	m_load = 0.0;
	m_layout = layout; // Attribut speichert layout

	CCabinDoor LeftDoor(CDoor::LEFT);
	CCabinDoor RightDoor(CDoor::RIGHT);

	switch (m_layout)
	{
	case CCabinDoor::LEFT:
	{
		m_cabinDoors[CDoor::LEFT] = LeftDoor;
		break;
	}
	case CCabinDoor::RIGHT:
	{
		m_cabinDoors[CDoor::RIGHT] = RightDoor;
		break;
	}
	case CCabinDoor::BOTH:
	{
		m_cabinDoors[CDoor::LEFT] = LeftDoor;
		m_cabinDoors[CDoor::RIGHT] = RightDoor;
		break;
	}
	default:
		break;
	}
}

/*! \fn void CCabin::setup()
 *  \brief Verbindet die kabineninternen Sensoren und Aktoren mit dem Kabinencontroller und den
 *   physikalischen Groessen der Kabine (z. B. Hoehensensor mit der Kabinenhoehe usw.) und
 *   verbindet im Gegenzug den Kabinencontroller mit den Sensoren und Aktoren. 
 */
void CCabin::setup(unsigned short numFloors)
{
	m_heightSensor.connect(&m_cabinController, &m_height);
	m_motor.connect(&m_cabinController, &m_height, &m_isDriving);
	m_timer.connect(&m_cabinController);
	m_cabinController.connectHeightSensor(&m_heightSensor);
	m_cabinController.connectMotor(&m_motor);
	m_cabinController.connectTimer(&m_timer);

	// connect den CabinController mit dem entsprechenden CabinDoor
	std::map<CDoor::SIDE, CCabinDoor>::iterator search_L = m_cabinDoors.find(
			CDoor::LEFT);

	if (search_L == m_cabinDoors.end()) //prueft ob die linke Kabine nicht in der map ist
	{
		cout << "Die  KabinenTuer ist nicht in der Map hinterlegt" << endl;
	}
	else
	{
		m_cabinController.connectCabinDoor(&m_cabinDoors[CDoor::LEFT]); //connect den CabinController mit dem linken CabinDoor
		m_cabinDoors[CDoor::LEFT].connect(&m_cabinController); //connect den linken CabinDoor mit dem CabinController
	}
	std::map<CDoor::SIDE, CCabinDoor>::iterator search_R = m_cabinDoors.find(
			CDoor::RIGHT);

	if (search_R == m_cabinDoors.end()) //prueft ob die rechte  Kabine nicht in der map ist
	{
		cout << "Die  KabinenTuer ist nicht in der Map hinterlegt" << endl;
	}
	else
	{
		m_cabinController.connectCabinDoor(&m_cabinDoors[CDoor::RIGHT]); //connect den CabinController mit dem rechten CabinDoor
		m_cabinDoors[CDoor::RIGHT].connect(&m_cabinController); //connect den rechten CabinDoor mit dem CabinController
	}

	m_cabinPanel.setNumFloors(numFloors); //Anzahl der Stockwerk setzen
	m_cabinPanel.connect(&m_cabinController); //connect den CabinPanel mit dem CabinController
	m_cabinController.connectCabinPanel(&m_cabinPanel); //connect den CabinController mit dem CabinPanel

}

/*! \fn CCabinControllerProxy CCabin::cabinControllerProxy()
 *  \brief Gibt ein Proxy-Objekt zurueck, das dem Systemcontroller gestattet,
 *  auf den Kabinencontroller zuzugreifen
 *  \return Ein Kabinencontroller-Proxy
 */
CCabinControllerProxy CCabin::cabinControllerProxy()
{
	return CCabinControllerProxy(&m_cabinController);
}

/*! \fn  CCabin::addEntrance(unsigned short floorNumber, CDoor::SIDE entranceSide)
 *  \brief intanzieren eines neuen CEtrance mit der uebegenenen Stokwerknummer auf der Zugangsseite,
 *  der neue zugang zur Map hinzufuegen, verbindet des soebenen hinzugefuegtes map-Eintrages mit dem Kabinencontroller
 *  und verbindet des Kabincontrollers mit dem soeben hinzugefuegten Map-Eintrag
 */
void CCabin::addEntrance(unsigned short floorNumber, CDoor::SIDE entranceSide)
{
	CEntrance entrance(floorNumber, entranceSide);
	m_entrances[floorNumber] = entrance;
	m_entrances[floorNumber].connect(&m_cabinController);
	m_cabinController.connectEntrance(&m_entrances[floorNumber]);
}
