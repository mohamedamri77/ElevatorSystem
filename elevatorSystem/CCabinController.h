/** \file CCabinController.h
 * \brief CCabinController class header file.
 * \details Enthaelt die Deklaration der Klasse CCabinController.
 * \author Reimund
 * \date 2016
 */

#ifndef CABINCONTROLLER_H_
#define CABINCONTROLLER_H_
#define DOOR_OPEN_DELAY 3000

#include "CEventQueue.h"
#include "CDoor.h"
#include "CCabinDoor.h"
#include "CCabinPanel.h"
#include "CEntrance.h"
#include <map>
#include <vector>
#include <iostream>
#include <list>

class CSystemController;
class CMotor;
class CTimer;
class CHeightSensor;

 /*! \class CCabinController
  * \brief Modelliert einen Kabinencontroller. Er ist verbunden mit den Komponenten
  *  der Kabine. Er steuert diese an, kann ihren Zustand abfragen und erhaelt Events
  *  von diesen. Er beherbergt die aufzugspezifische Zustandssteuerung.
  */
 class CCabinController
 {
     friend class CSimulator; /**< Simulator hat Vollzugriff auf private-Sektion
		dieser Klasse */
 public:
     CCabinController();

     void connectSystemController(CSystemController* pSystemController);
     void connectTimer(CTimer* pTimer);
     void connectMotor(CMotor* pMotor);
     void connectHeightSensor(CHeightSensor* pHeightSensor);
     void connectCabinDoor(CCabinDoor* pCabinDoor);
     void connectCabinPanel(CCabinPanel* pCabinPanel);
     void connectEntrance(CEntrance* pEntrances);
     void driveToFloor(unsigned short destinationFloorNumber);
     unsigned int currentFloor();

     void pushEvent(SEvent event);
     void work();
     void openAllDoorsCurrentFloor();
     void closeAllDoorsCurrentFloor();
     void addToAscendingList(unsigned int floor);
     void addToDescendingList(unsigned int floor);
     bool allDoorsClosedCurrentFloor();
     bool allDoorsOpenCurrentFloor();

 private:
 	CSystemController* m_pSystemController; 			/**< Pointer auf den uebergeordneten Systemcontroller */
 	CMotor* m_pMotor; 									/**< Pointer auf den Motor */
 	CTimer* m_pTimer; 									/**< Pointer auf den Timer */
 	CHeightSensor* m_pHeightSensor;						/**< Pointer auf den Hoehensensor */
    CEventQueue m_eventQueue;							/**< Queue der aufgelaufenen Events */
    enum STATE_TYPE {
    	IDLE_DOORS_CLOSED, 			/**< Startzustand, beide Listen leer, Aufzug steht,  Türen geschlossen */
		IDLE_DOORS_OPENING,			/**< Beide Listen leer, Aufzug steht, Türen öffnen gerade */
		IDLE_DOORS_OPEN,			/**< Beide Listen leer, Aufzug steht,Türen offen  */
    	IDLE_DOORS_CLOSING,			/**< Beide Listen leer, Aufzug steht, Türen schließen gerade */
		ASCENDING_DRIVING,			/**< Aufsteigende Liste  wird abgearbeitet, Aufzug  fährt nach oben, Türen geschlossen  */
		ASCENDING_DOORS_OPENING,	/**< Aufsteigende Liste wird abgearbeitet, Aufzug steht,  Türen öffnen gerade */
		ASCENDING_DOORS_OPEN,		/**< Aufsteigende Liste wird abgearbeitet, Aufzug steht, Türen offen  */
		ASCENDING_DOORS_CLOSING,	/**< Aufsteigende Liste wird abgearbeitet, Aufzug steht,  Türen schließen gerade */
		DESCENDING_DRIVING,			/**< Absteigende Liste wird abgearbeitet, Aufzug fährt nach unten, Türen geschlossen  */
		DESCENDING_DOORS_OPENING,	/**< Absteigende Liste wird abgearbeitet, Aufzug steht,  Türen öffnen gerade */
		DESCENDING_DOORS_OPEN,		/**< Absteigende Liste wird abgearbeitet, Aufzug steht, Türen offen  */
		DESCENDING_DOORS_CLOSING,	/**< Absteigende Liste wird abgearbeitet, Aufzug steht,  Türen schließen gerade  */
		FIREALARM_DRIVING,			/**< Im Brandfall sollen alle Aufzuege, die gerade fahren (ASCENDING_DRIVING oder DESCENDING_ DRIVING), ihren aktuellen Befoerderungsauftrag abfertigen. */
		FIREALARM_FINAL				/**< Endzustand des Feueralarms: Bei geoeffneten Tueren stehen bleiben und auf keine Eingaben mehr reagieren. */
    };
    STATE_TYPE m_stateType;								/**< Zustand des Cabins. */
    CCabinPanel* m_pCabinPanel;							/**< Pointer auf den Kabinenpanel*/
    std::map<CDoor::SIDE, CCabinDoor*> m_pCabinDoors;	/**< Map-Verzeichnis der Kabinentueren*/
    std::map<unsigned short, CEntrance*> m_pEntrances;	/**< Map-Verzeichnis der Zugaenge*/
    unsigned int m_targetFloor;							/**< Stockwerknummer*/
    std::list<unsigned int> m_ascendingList;			/**< aufsteigende Liste beim Hochfahren anzufahrender Stockwerke*/
    std::list<unsigned int> m_descendingList;			/**< absteigende Liste beim Herunterfahren anzufahrender Stockwerke*/

 };

 #endif /* CABINCONTROLLER_H_ */
