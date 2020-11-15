/** \file CCabinDoor.cpp
 * \brief CCabinDoor class source file.
 * \details Enthaelt die Implementierung der Klasse CCabinDoor.
 * \author Reimund
 * \date 2016
 */

#include "CCabinDoor.h"
#include "CCabinController.h"
#include "SEvent.h"


/*! \fn CCabinDoor::CCabinDoor(CDoor::SIDE side)
 *  \brief Konstruktor. Legt die Tuerseite fest, intitialisiert alle anderen
 *  Attribute sowie die Pointer mit 0. Zu Beginn ist die Kabinentuer geschlossen 
 *  und nicht blockiert
 */
CCabinDoor::CCabinDoor(CDoor::SIDE side)
{
	//Labor2_Vor
	m_unblocked = true;			//Die Kabinentuer ist nicht blockiert
	if(side == CDoor::RIGHT) {
		m_side = CDoor::RIGHT;	//Rechte Tuerseite festlegen
	} else {
		m_side = CDoor::LEFT;	//Linke Tuerseite festlegen
	}
	m_step = 0;					//Interner Simulationsschrittzaehler startet bei 0
	m_state = IS_CLOSED;		//Die Kabinentuer ist geschlossen
	m_pCabinController = 0;		//Pointer mit 0 initialisieren
}

/*! \fn void CCabinDoor::connect(CCabinController* pCabinController)
 *  \brief Verbindet die Kabinentuer mit dem uebergeordneten Kabinencontroller
 *  \param pCabinController Pointer auf den uebergeordneten Kabinencontroller
 */
void CCabinDoor::connect(CCabinController* pCabinController)
{
	//Labor2_Vor
	m_pCabinController = pCabinController;  //Kabinentuer wurde zu Kabinenkontroller verbunden
}

/*! \fn CCabinDoor::~CCabinDoor()
 *  \brief Destruktor (leer)
 */
CCabinDoor::~CCabinDoor()
{

}

/*! \fn void CCabinDoor::unblocked()
 *  \brief Liefert zurueck, ob die Lichtschranke frei ist
 *  \return true, falls die Lichtschranke frei ist, false andernfalls
 */
bool CCabinDoor::unblocked()
{
	return m_unblocked;
}

/*! \fn void CCabinDoor::evDoorBlocked()
 *  \brief Loest das Event "Lichtschranke wurde gerade blockiert" aus
 */
void CCabinDoor::evDoorBlocked()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=DOOR_BLOCKED;			//
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}

/*! \fn void CCabinDoor::evDoorBlocked()
 *  \brief Loest das Event "Lichtschranke jetzt wieder frei" aus
 */
void CCabinDoor::evDoorUnblocked()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=DOOR_UNBLOCKED;		//Eventtyp als DOOR_UNBLOCKED zuweisen
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}

/*! \fn void CCabinDoor::evDoorOpens()
 *  \brief Loest das Event "Kabinentuer oeffnet jetzt" aus
 */
void CCabinDoor::evDoorOpens()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=CABINDOOR_OPENS;
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}

/*! \fn void CCabinDoor::evDoorCloses()
 *  \brief Loest das Event "Kabinentuer schliesst jetzt" aus
 */
void CCabinDoor::evDoorCloses()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=CABINDOOR_CLOSES;
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}

/*! \fn void CCabinDoor::evDoorFullyOpen()
 *  \brief Loest das Event "Kabinentuer jetzt vollstaendig offen" aus
 */
void CCabinDoor::evDoorFullyOpen()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=CABINDOOR_FULLY_OPEN;
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}

/*! \fn void CCabinDoor::evDoorFullyClosed()
 *  \brief Loest das Event "Kabinentuer jetzt vollstaendig geschlossen" aus
 */
void CCabinDoor::evDoorFullyClosed()
{
    SEvent newEvent;							//neue Ereignis von struct SEvent erzeugen
    newEvent.m_eventType=CABINDOOR_FULLY_CLOSED;
    newEvent.m_additionalInfo=(unsigned long long)this;
    m_pCabinController->pushEvent(newEvent);	//Das erstellte newEvent wird durch pushEvent versendet.
}
