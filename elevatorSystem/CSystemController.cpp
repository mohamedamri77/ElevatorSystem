/** \file CSystemController.cpp
 * \brief CSystemController class source file.
 * \details Enthaelt die Implementierung der Klasse CSystemController.
 * \author Reimund
 * \date 2016
 */

#include "CSystemController.h"
#include "CCabinController.h"
#include "CCabin.h"

/*! \fn void CSystemController::work()
 *  \brief Laesst den Systemcontroller einen Simulationsschritt
 *   weiterarbeiten. Hier ist der Zustandsautomat fuer die uebergeordnete
 *   Systemsteuerung zu implementieren. Die Methode wird periodisch
 *   durch den Simulator aufgerufen
 */
void CSystemController::work()
{

}

/*! \fn CSystemController::CSystemController()
 *  \brief Konstruktor; Der Systemcontroller startet im Zustand NORMAL
 */
CSystemController::CSystemController()
{
	m_state = NORMAL;
}

/*! \fn void CSystemController::connectCabinController(CCabinControllerProxy cabinControllerProxy)
 *  \brief Stellt die beiderseitige Verbindung zwischen System- und dem uebergebenen Cabincontroller her
 *  \param cabinControllerProxy Proxy-Objekt, das dem Systemcontroller
 *  Zugriff auf den zu verbindenden Kabinencontroller gestattet
 */
void CSystemController::connectCabinController(CCabinControllerProxy cabinControllerProxy)
{
	m_pCabinControllers.push_back(cabinControllerProxy.m_pCabinController);
	cabinControllerProxy.m_pCabinController->connectSystemController(this);
}

/*! \fn void CSystemController::pushEvent(SEvent event)
	\brief Fuegt dem Kabinencontroller ein Event hinzu
	\param event Das hinzuzufuegende Event
 */
void CSystemController::pushEvent(SEvent event)
{
    printEvent(event, (void*)this);
	m_eventQueue.pushEvent(event);
}
