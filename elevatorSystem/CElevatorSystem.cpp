/** \file CElevatorSystem.cpp
 * \brief CElevatorSystem class source file.
 * \details Enthaelt die Implementierung der Klasse CElevatorSystem.
 * \author Reimund
 * \date 2016
 */

#include "CElevatorSystem.h"
#include "CCabin.h"
#include <iostream>
#include <list>

/*! \fn CElevatorSystem::CElevatorSystem(unsigned short numFloors)
 *  \brief Konstruktor; Legt die Anzahl der Stockwerke fuer das Aufzugsystem fest.
 *   MAX_FLOORS darf nicht ueberschritten und 2 nicht unterschritten werden
 *   \param numFloors Anzahl der Stockwerke
 */
 CElevatorSystem::CElevatorSystem(unsigned short numFloors)
 {
     if(numFloors>MAX_FLOORS)
     {
         std::cerr << "ERROR: CElevatorSystem(...): Max amount of floors is " << MAX_FLOORS
             << ", set to that\n" << std::flush;
         numFloors=MAX_FLOORS;
     }
     if(numFloors<2)
     {
         std::cerr << "ERROR: CElevatorSystem(...): Min amount of floors is 2, set to that\n"
             << std::flush;
         numFloors=2;
     }
     m_numFloors=numFloors;
 }

 /*! \fn void CElevatorSystem::addElevator()
  *  \brief Fuegt dem Aufzugsystem einen Aufzug
  *   hinzu und verbindet den Systemcontroller mit dem hinzugekommenen Aufzugcontroller
  *   \param doorLayout Tuerenlayout
  */
 void CElevatorSystem::addElevator(CCabinDoor::LAYOUT doorLayout)
 {
     if((m_cabins.size()+1) <= MAX_ELEVATORS)
     {
         CCabin newCabin(doorLayout);		//neue Kabine erzeugen und das Tuerlayout uebergeben
         m_cabins.push_back(newCabin);		//die neue Kabine in eine Liste names m_cabins hinten anfuegen
         m_cabins.back().setup(m_numFloors);
         m_systemController.connectCabinController(m_cabins.back().cabinControllerProxy());
     }
     else
     {
         std::cerr << "ERROR: CElevatorSystem::addElevator() Max amount of elevators reached ("
         << MAX_ELEVATORS << ")\n" << std::flush;
     }
     return;
}

 /*! \fn void CElevatorSystem::addEntrance(unsigned short elevatorNumber,
		unsigned short floorNumber, CDoor::SIDE entranceSide)
   *  \brief Fuegt dem Aufzugsystem einen Zugang hinzu.
   */
void CElevatorSystem::addEntrance(unsigned short elevatorNumber,
		unsigned short floorNumber, CDoor::SIDE entranceSide)
{
	std::list<CCabin>::iterator cabinIterator;		// Ein Iterator namens cabinIterator, der über eine Liste von Objekten des Typs CCabin iterieren kann
	cabinIterator = m_cabins.begin();				// Der Iterator wird der Anfang der Liste m_cabins zugewiesen
	std::advance(cabinIterator, elevatorNumber);	// Der Iterator wird auf Aufzugsnummer weiter  gerückt, sodass er jetzt
													//auf das naechste Element zeigt, das der Liste hinzugefügt  wurde
	(*cabinIterator).addEntrance(floorNumber,entranceSide); // Es wird die Methode addEntrance() des Objekts aufgerufen, auf das cabinIterator gerade zeigt
}
