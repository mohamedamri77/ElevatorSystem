/** \file CCabinController.cpp
 * \brief CCabinController class source file.
 * \details Enthaelt die Implementierung der Klasse CCabinController.
 * \author Reimund
 * \date 2016
 */

#include "CCabinController.h"
#include "CSystemController.h"
#include "CMotor.h"
#include "CTimer.h"
#include "CHeightSensor.h"

/*! \fn CCabinController::work()
 *  \brief Laesst den Kabinencontroller einen Simulationsschritt
 *  weiterarbeiten. Hier ist der Zustandsautomat fuer den einzelnen
 *  Aufzug zu implementieren. Die Methode wird periodisch durch den
 *  Simulator aufgerufen. Durch eine 'static' Variable wuerde die Variable
 *   von allen Objekten der Klasse verwendet werden. Aber hier braucht
 *    jeder Aufzug einen eigenen Zustand.
 */

void CCabinController::work()
{
	unsigned int floor = 0;
	std::vector<SEvent> events=m_eventQueue.popEvents();
	for(unsigned int i=0; i<events.size(); i++)         // Neue Events abarbeiten
	{
		SEvent ev=events[i];                            // Event Nr. i abarbeiten
		switch(m_stateType)
		{
		case IDLE_DOORS_CLOSED: // ###################################################                  //falls aktueller Zustand "wartend, tueren zu
		{
			std::cout << "In state IDLE_DOORS_CLOSED: " << std::flush;
			switch(ev.m_eventType)                                                                      //Aktuellen Zustand prufen
			{
			case KEY_FLOOR:
			case KEY_UP:
			case KEY_DOWN:
			{
				floor=ev.m_additionalInfo;                                                              // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                          // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                 // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                               // LED Runter-Taste an


				if(floor == currentFloor())
				{
					openAllDoorsCurrentFloor();                                                         //alle Tueren im aktuellen Stockwerk oeffnen
					m_stateType=IDLE_DOORS_OPENING;                                                     //in den Zustand "wartend, Tueren oeffnen gerade" wechseln
					m_pCabinPanel->m_floorKeySignals[floor]=0;
					m_pCabinPanel->m_closeDoorsKeySignal=0;
					m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                          //zur Liste der aufsteigenden Sortierungen hinzufügen.
					driveToFloor(floor);                                                                //fahre zum gerufenen Stockwerk
					m_stateType=ASCENDING_DRIVING;                                                      //in den Zustand "Aufwärtsfahrt" wechseln
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                         //zur Liste der absteigenden Sortierungen hinzufügen.
					driveToFloor(floor);                                                                //fahre zum gerufenen Stockwerk
					m_stateType=DESCENDING_DRIVING;                                                     //in den Zustand "Abwärtsfahrt" wechseln
				}

				break;
			}
			case KEY_OPEN_DOORS:
			{
				openAllDoorsCurrentFloor();                                                             //alle Tueren im aktuellen Stockwerk oeffnen
				m_pCabinPanel->m_closeDoorsKeySignal=0;                                                 // LED Schließ-Taste aus
				m_pCabinPanel->m_openDoorsKeySignal=1;                                                  //LED Öffnen-Taste an
				m_stateType=IDLE_DOORS_OPENING;                                                         //in den Zustand "wartend, Tueren oeffnen gerade" wechseln


				break;
			}
			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				openAllDoorsCurrentFloor();
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}

			default:
			{
				break;
			}

			}
			break;
		}

		case IDLE_DOORS_OPENING: // ###################################################                 //falls aktueller Zustand "wartend, Tueren oeffnen gerade
		{
			m_pCabinPanel->m_openDoorsKeySignal = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal = false;
			std::cout << "In state IDLE_DOORS_OPENING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                              // Speichere gerufenen Stockwerk ab
				m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                          //zur Liste der aufsteigenden Sortierungen hinzufügen.
					closeAllDoorsCurrentFloor();                                                        //alle Tueren im aktuellen Stockwerk schliessen
					m_stateType = ASCENDING_DOORS_CLOSING;                                              //in den Zustand "Aufwärtsfahrt, Tueren schließen" wechseln.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                         //zur Liste der absteigenden Sortierungen hinzufügen.
					closeAllDoorsCurrentFloor();                                                        //alle Tueren im aktuellen Stockwerk schliessen
					m_stateType = DESCENDING_DOORS_CLOSING;                                             //in den Zustand "Abwärtsfahrt, Tueren schließen" wechseln.
				}

				break;
			}
			case KEY_UP:       // oder
			case KEY_DOWN:
			{

				floor=ev.m_additionalInfo;                                                              // Speichere gerufenen Stockwerk ab

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                 // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                               // LED Runter-Taste an


				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                          //zur Liste der aufsteigenden Sortierungen hinzufügen.
					m_stateType = ASCENDING_DOORS_OPENING;                                              //in den Zustand "Aufwärtsfahrt, Tueren oeffnen gerade" wechseln.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                         //zur Liste der absteigenden Sortierungen hinzufügen.
					m_stateType = DESCENDING_DOORS_OPENING;                                             //in den Zustand "Abwärtsfahrt, Tueren oeffnen gerade" wechseln.
				}

				break;
			}
			case KEY_CLOSE_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal= 1;                                                // LED Schließ-Taste an
				m_pCabinPanel->m_openDoorsKeySignal= 0;                                                 //LED Öffnen-Taste aus
				closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = IDLE_DOORS_CLOSING;                                                       //in den Zustand "wartend, Tür geschlossen" wechseln

				break;
			}
			case CABINDOOR_FULLY_OPEN:
			case ENTRANCEDOOR_FULLY_OPEN:
			{
				if(allDoorsOpenCurrentFloor()==true)                                                    //wenn alle Tueren offen sind
				{
					m_pCabinPanel->m_openDoorsKeySignal=0;                                              //LED Öffnen-Taste aus
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =0;                               //LED Runter-Taste aus
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =0;                                 // LED Hoch-Taste an
					m_pTimer->set(DOOR_OPEN_DELAY);                                                     //setze Timer auf DELAY-Zeit
					m_stateType = IDLE_DOORS_OPEN;                                                      //in den Zustand "wartend, tueren offen" wechseln
				}

				break;
			}
			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				openAllDoorsCurrentFloor();
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case IDLE_DOORS_OPEN: // ###################################################                    //falls aktueller Zustand "wartend, Tueren offen"
		{
			std::cout << "In state IDLE_DOORS_OPEN: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:      // oder
			case KEY_DOWN:
			{

				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab

				if(ev.m_eventType == KEY_UP)
				{
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(ev.m_eventType == KEY_DOWN)
				{
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(floor==currentFloor())
				{
					m_pTimer->set(DOOR_OPEN_DELAY);                                                         //setze Timer auf DELAY-Zeit
					m_pCabinPanel->m_floorKeySignals[floor]=0;
				}
				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
					m_stateType = ASCENDING_DOORS_OPEN;                                                     //in den Zustand "Aufwärtsfahrt, Tueren oeffnen" wechseln.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
					m_stateType = DESCENDING_DOORS_OPEN;                                                    //in den Zustand "Abwärtsfahrt, Tueren oeffnen" wechseln.
				}

				break;
			}
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				m_pCabinPanel->m_floorKeySignals[floor]=1;                                                  // LED Wahlstockwerk an

				if(floor==currentFloor())
				{
					m_pTimer->set(DOOR_OPEN_DELAY);
					m_pCabinPanel->m_floorKeySignals[floor]= 0;
				}
				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
					closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
					m_stateType = ASCENDING_DOORS_CLOSING;                                                  //in den Zustand "Aufwärtsfahrt, Tueren schließen" wechseln.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
					closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
					m_stateType = DESCENDING_DOORS_CLOSING;                                                 //in den Zustand "Abwärtsfahrt, Tueren schließen" wechseln.
				}


				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				m_pTimer->set(DOOR_OPEN_DELAY);                                                             //setze Timer auf DELAY-Zeit

				break;
			}
			case TIMER:
			case KEY_CLOSE_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal = 1;                                                   // LED Schließ-Taste an

					closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = IDLE_DOORS_CLOSING;                                                           //in den Zustand "wartend, Tür geschlossen" wechseln

				break;
			}
			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case IDLE_DOORS_CLOSING: // ###################################################                     // Falls in den Zustand "wartend, Tür geschlossen"
		{
			m_pCabinPanel->m_closeDoorsKeySignal = false;
			std::cout << "In state IDLE_DOORS_CLOSING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
				{
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an
					//m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(ev.m_eventType == KEY_DOWN)
				{
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an
					//m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(floor == currentFloor())
				{
					openAllDoorsCurrentFloor();                                                             //alle Tueren im aktuellen Stockwerk oeffnen
					m_stateType=IDLE_DOORS_OPENING;                                                         //in den Zustand "wartend, Tueren oeffnen gerade" wechseln
					m_pCabinPanel->m_floorKeySignals[floor]=0;
					m_pCabinPanel->m_closeDoorsKeySignal=0;
					m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an

				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
					m_stateType=ASCENDING_DOORS_CLOSING;                                                    //in den Zustand "Aufwärtsfahrt, Tueren schließen" wechseln.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
					m_stateType=DESCENDING_DOORS_CLOSING;                                                   //in den Zustand "Abwärtsfahrt, Tueren schließen" wechseln.
				}


				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal=0;
				openAllDoorsCurrentFloor();                                                                 //alle Tueren im aktuellen Stockwerk oeffnen
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				m_stateType = IDLE_DOORS_OPENING;                                                           //in den Zustand "wartend, Tueren oeffnen gerade" wechseln

				break;
			}
			case CABINDOOR_FULLY_CLOSED:    // oder
			case ENTRANCEDOOR_FULLY_CLOSED:
			{
				if(allDoorsClosedCurrentFloor() == true)                                                    //wenn alle Tueren zu sind
				{
					m_stateType = IDLE_DOORS_CLOSED;                                                        //in den Zustand "wartend, Tür geschlossen" wechseln
					m_pCabinPanel->m_closeDoorsKeySignal = 0;                                               // LED Schließ-Taste zu
				}
				break;
			}


			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				openAllDoorsCurrentFloor();
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}

			default:
			{
				break;
			}
			}
			break;
		}

		case ASCENDING_DRIVING: // ###################################################
		{
			std::cout << "In state ASCENDING_DRIVING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:      // oder
			case KEY_DOWN:    // oder
			case KEY_FLOOR:
			{
				if(ev.m_eventType == KEY_UP)
				{
					floor = ev.m_additionalInfo;								//speichert aufgerufenen Stockwerk
					m_pEntrances[floor]->m_entrancePanel.m_upSignal=true;     //LED leuchtet
				}
				if(ev.m_eventType == KEY_DOWN)
				{
					floor = ev.m_additionalInfo;								//speichert aufgerufenen Stockwerk
					m_pEntrances[floor]->m_entrancePanel.m_downSignal=true;   //LED leuchtet
				}
				if(ev.m_eventType == KEY_FLOOR)
				{
					m_pCabinPanel->m_floorKeySignals[ev.m_additionalInfo]=true;	//LED leuchtet
					floor = ev.m_additionalInfo;								//speichert aufgerufenen Stockwerk
				}
				if (floor <= currentFloor())
				{
					addToDescendingList(floor);								//fuegt Floornummer zur absteigende Liste hinzu
				}
				if (floor > currentFloor())
				{
					addToAscendingList(floor);								//fuegt Floornummer zur aufsteigende Liste hinzu
					driveToFloor(m_ascendingList.front());						//faehrt zum Wunschstockwerk
				}
				break;
			}
			case MOTOR_STOPPED:
			{
				m_pCabinPanel->m_floorKeySignals[m_ascendingList.front()] = false;	//LED der Stockwerkwahltaste aus
				m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal = false;	//LED "up" auf aktuellem Stockwerkseingang aus
				if(m_descendingList.empty())										//wenn absteigende Liste leer
				{
					m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal = false; //LED "down" aus
				}
				openAllDoorsCurrentFloor();											//oeffnet alle Tuere
				m_ascendingList.pop_front();										//loesche ersten Antrag aus der aufsteigenden Liste
				m_stateType = ASCENDING_DOORS_OPENING;				//wechselt in FIREALARM_FINAL-Zustand
				break;
			}
			case REACHED_FLOOR:
			{
				m_pCabinPanel->m_currentFloorSignal = currentFloor();   //Anzeige des Zustands des Aufzugs im Kabinpanel.
				for(std::map<unsigned short, CEntrance*>::iterator it=m_pEntrances.begin(); it!=m_pEntrances.end(); ++it)   //Liste der Kabineneingaenge durchgehen
				{
					m_pEntrances[it->first]->m_entrancePanel.m_currentFloorSignal = currentFloor(); //Auf jedem Eingangspanel Stockwerk anzeigen
				}
				break;
			}
			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);								//sendet Event zu System Controller
				m_pTimer->set(500);												//setzt Timer
				m_stateType = FIREALARM_DRIVING;				//wechselt in FIREALARM_DRIVING-Zustand
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case ASCENDING_DOORS_OPENING: // ###################################################                //Falls in den Zustand "Aufwärtsfahrt, Tueren oeffene gerade"
		{
			m_pCabinPanel->m_floorKeySignals[currentFloor()] = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal = false;

			std::cout << "In state ASCENDING_DOORS_OPENING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:     // oder
			case KEY_DOWN:   // oder
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
				{
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(ev.m_eventType == KEY_DOWN)
				{
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}


				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.

				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.

				}

				break;
			}
			case KEY_CLOSE_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal = 1;                                                   // LED Schließ-Taste an
				closeAllDoorsCurrentFloor();                                                                //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = ASCENDING_DOORS_CLOSING;                                                      //in den Zustand "Aufwärtsfahrt, Tueren schließen" wechseln.

				break;
			}
			case CABINDOOR_FULLY_OPEN:    // oder
			case ENTRANCEDOOR_FULLY_OPEN:
			{
				if(allDoorsOpenCurrentFloor() == true)                                                      //wenn alle Tueren auf sind
				{
					m_pTimer->set(DOOR_OPEN_DELAY);                                                         //setze Timer auf DELAY-Zeit
					m_stateType = ASCENDING_DOORS_OPEN;                                                     //in den Zustand "Aufwärtsfahrt, Tueren oeffenen" wechseln.
					m_pCabinPanel->m_openDoorsKeySignal = 0;                                                //LED Öffnen-Taste zu
				}

				break;
			}
			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}

			default:
			{
				break;
			}
			}
			break;
		}

		case ASCENDING_DOORS_OPEN: // ##############################                                        //Falls in den Zustand "Aufwärtsfahrt, Tueren oeffene"
		{

			std::cout << "In state ASCENDING_DOORS_OPEN: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:       // oder
			case KEY_DOWN:     // oder
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
				{
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(ev.m_eventType == KEY_DOWN)
				{
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an
					m_pCabinPanel->m_floorKeySignals[floor]=1;
				}

				if(floor ==currentFloor())
				{
					m_pTimer->set(DOOR_OPEN_DELAY);                                                         //setze Timer auf DELAY-Zeit
					m_pCabinPanel->m_floorKeySignals[floor]=0;
				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
				}


				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pTimer->set(DOOR_OPEN_DELAY);                                                             //setze Timer auf DELAY-Zeit
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an


				break;
			}
			case TIMER:
			case KEY_CLOSE_DOORS:
			{

					closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = ASCENDING_DOORS_CLOSING;                                                      //in den Zustand "Aufwärtsfahrt, Tueren schließen" wechseln.
				m_pCabinPanel->m_closeDoorsKeySignal = 1;                                                   // LED Schließ-Taste an

				break;
			}

			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case ASCENDING_DOORS_CLOSING: // ###################################################                // Falls in den Zustand "Aufwärtsfahrt, Tueren schließen"
		{
			std::cout << "In state ASCENDING_DOORS_CLOSING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:      // oder
			case KEY_DOWN:    // oder
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an

				if(floor ==currentFloor())
				{
					m_pCabinPanel->m_floorKeySignals[currentFloor()] = false;
					m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal =
							false;
					m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal =
							false;
					openAllDoorsCurrentFloor();                                                             //alle Tueren im aktuellen Stockwerk oeffnen
					m_stateType= ASCENDING_DOORS_OPENING;                                                   //in den Zustand "Aufwärtsfahrt, Tueren oeffenen gerade" wechseln.
					m_pCabinPanel->m_floorKeySignals[floor]=0;
					m_pCabinPanel->m_closeDoorsKeySignal=0;
					m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
				}


				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal=0;
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				openAllDoorsCurrentFloor();                                                                 //alle Tueren im aktuellen Stockwerk oeffnen
				m_stateType = ASCENDING_DOORS_OPENING;                                                      //in den Zustand "Aufwärtsfahrt, Tueren oeffenen gerade" wechseln.
				break;
			}
			case CABINDOOR_FULLY_CLOSED:    // oder
			case ENTRANCEDOOR_FULLY_CLOSED:
			{
				if(allDoorsClosedCurrentFloor()  && !m_ascendingList.empty())                               //wenn alle Turen zu und aufsteigende Liste nicht leer
				{
					driveToFloor(m_ascendingList.front());                                                  //fahre zum naechsten Stockwerk in der aufsteigenden liste
					m_stateType= ASCENDING_DRIVING;                                                         //in den Zustand "Aufwärtsfahrt, Fahrend" wechseln
				}

				if(allDoorsClosedCurrentFloor()  && m_ascendingList.empty() && !m_descendingList.empty())   //wenn alle Turen zu und aufsteigende Liste leer und absteigende Liste nicht leer
				{
					driveToFloor(m_descendingList.front());                                                 //fahre zum naechsten Stockwerk in der absteigenden liste
					m_stateType= DESCENDING_DRIVING;                                                        //in den Zustand "Abwärtsfahrt, Fahrend" wechseln
				}

				if(allDoorsClosedCurrentFloor()  && m_ascendingList.empty() && m_descendingList.empty())    //wenn alle Turen zu und aufsteigende Liste leer und absteigende Liste leer
				{
					m_stateType= IDLE_DOORS_CLOSED;                                                         //in den Zustand "wartend, Tür geschlossen" wechseln
				}

				m_pCabinPanel->m_closeDoorsKeySignal =0;                                                    // LED Schließ-Taste zu
				break;
			}


			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				openAllDoorsCurrentFloor();
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case DESCENDING_DRIVING: // ###################################################                     // Falls in den Zustand "Abwärtsfahrt, Fahrend"
		{
			std::cout << "In state DESCENDING_DRIVING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab

				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an


				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.

				}

				if(floor <= currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
					driveToFloor(m_descendingList.front());                                                 //fahre zum naechsten Stockwerk in der absteigenden Liste

				}
				break;
			}
			case MOTOR_STOPPED:
			{

				openAllDoorsCurrentFloor();                                                                 //alle Tueren im aktuellen Stockwerk oeffnen
				m_descendingList.pop_front();                                                               // loesche ersten Antrag aus der aufsteigenden Liste
				m_stateType =DESCENDING_DOORS_OPENING;                                                      //in den Zustand "Abwärtsfahrt, Tueren oeffenen gerade" wechseln.
				m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal = 0;                             //LED Runter-Taste aus
				m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal = 0;                               // LED Hoch-Taste aus
				m_pCabinPanel->m_floorKeySignals[currentFloor()]=0;                                         // LED Wahlstockwerk aus
				m_pCabinPanel->m_closeDoorsKeySignal=0;
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an


				break;
			}
			case REACHED_FLOOR:                                                                        // Höhe-Sensor meldet erreichten Stockwerk

				m_pCabinPanel->m_currentFloorSignal = currentFloor();                                // Stockwerk Auf dem KabinenPanel anzeigen
				for(std::map<unsigned short , CEntrance*>:: iterator
						it = m_pEntrances.begin() ; it != m_pEntrances.end() ;++it)                    // Liste der Kabineneingänge druchgehen
				{
					m_pEntrances[it->first]->m_entrancePanel.m_currentFloorSignal = currentFloor();     // erreichte Stockwerk auf dem ZugangPanel anzeigen
				}
				break;

			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_DRIVING;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case DESCENDING_DOORS_OPENING: // ###################################################               //Falls in den Zustand "Abwärtsfahrt, Tueren oeffenen gerade"
		{
			std::cout << "In state DESCENDING_DOORS_OPENING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:       // oder
			case KEY_DOWN:     // oder
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab
				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an


				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.

				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.

				}


				break;
			}
			case KEY_CLOSE_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal = 1;                                                   // LED Schließ-Taste an
				closeAllDoorsCurrentFloor();                                                                //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = DESCENDING_DOORS_CLOSING;                                                     //in den Zustand "Abwärtsfahrt, Tueren schließen" wechseln.


				break;
			}
			case CABINDOOR_FULLY_OPEN:
			case ENTRANCEDOOR_FULLY_OPEN:
			{
				if(allDoorsOpenCurrentFloor() == true)
				{
					m_pTimer->set(DOOR_OPEN_DELAY);                                                         //setze Timer auf DELAY-Zeit
					m_stateType = DESCENDING_DOORS_OPEN;                                                    //in den Zustand "Abwärtsfahrt, Tueren oeffenen" wechseln.
					m_pCabinPanel->m_openDoorsKeySignal = 0;                                                //LED Öffnen-Taste zu
				}

				break;
			}

			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case DESCENDING_DOORS_OPEN: // ###################################################                  // Falls in den Zustand "Abwärtsfahrt, Tueren oeffenen"
		{
			m_pCabinPanel->m_openDoorsKeySignal = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_downSignal = false;
			m_pEntrances[currentFloor()]->m_entrancePanel.m_upSignal = false;
			std::cout << "In state DESCENDING_DOORS_OPEN: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:       // oder
			case KEY_DOWN:     // oder
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab

				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an

				if(floor ==currentFloor())
				{
					m_pTimer->set(DOOR_OPEN_DELAY);                                                         //setze Timer auf DELAY-Zeit
					m_pCabinPanel->m_floorKeySignals[floor]=0;
				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
				}



				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pTimer->set(DOOR_OPEN_DELAY);                                                             //setze Timer auf DELAY-Zeit
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an


				break;
			}
			case TIMER:
			case KEY_CLOSE_DOORS:
			{


					closeAllDoorsCurrentFloor();                                                            //alle Tueren im aktuellen Stockwerk schliessen
				m_stateType = DESCENDING_DOORS_CLOSING;                                                     //in den Zustand "Abwärtsfahrt, Tueren schließen" wechseln
				m_pCabinPanel->m_closeDoorsKeySignal = 1;                                                   // LED Schließ-Taste an


				break;
			}

			case FIREALERT:
			{
				m_pSystemController->pushEvent(ev);
				m_stateType = FIREALARM_FINAL;
				m_pTimer->set(200);
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case DESCENDING_DOORS_CLOSING: // ###################################################               //Falls in den Zustand "Abwärtsfahrt, Tueren schließen"
		{
			std::cout << "In state DESCENDING_DOORS_CLOSING: " << std::flush;
			switch(ev.m_eventType)
			{
			case KEY_UP:
			case KEY_DOWN:
			case KEY_FLOOR:
			{
				floor=ev.m_additionalInfo;                                                                  // Speichere gerufenen Stockwerk ab

				if(ev.m_eventType ==KEY_FLOOR)
					m_pCabinPanel->m_floorKeySignals[floor]=1;                                              // LED Wahlstockwerk an

				if(ev.m_eventType == KEY_UP)
					m_pEntrances[floor]->m_entrancePanel.m_upSignal =1;                                     // LED Hoch-Taste an

				if(ev.m_eventType == KEY_DOWN)
					m_pEntrances[floor]->m_entrancePanel.m_downSignal =1;                                   // LED Runter-Taste an

				if(floor ==currentFloor())
				{
					openAllDoorsCurrentFloor();                                                             //alle Tueren im aktuellen Stockwerk oeffnen
					m_stateType= DESCENDING_DOORS_OPENING;                                                  //in den Zustand "Abwärtsfahrt, Tueren oeffenen gerade" wechseln.
					m_pCabinPanel->m_floorKeySignals[floor]=0;
					m_pCabinPanel->m_closeDoorsKeySignal=0;
					m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				}

				if(floor > currentFloor())
				{
					addToAscendingList(floor);                                                              //zur Liste der aufsteigenden Sortierungen hinzufügen.
				}

				if(floor < currentFloor())
				{
					addToDescendingList(floor);                                                             //zur Liste der absteigenden Sortierungen hinzufügen.
				}

				break;
			}
			case KEY_OPEN_DOORS:
			{
				m_pCabinPanel->m_closeDoorsKeySignal=0;
				m_pCabinPanel->m_openDoorsKeySignal = 1;                                                    //LED Öffnen-Taste an
				openAllDoorsCurrentFloor();                                                                 //alle Tueren im aktuellen Stockwerk oeffnen
				m_stateType = DESCENDING_DOORS_OPENING;                                                     //in den Zustand "Abwärtsfahrt, Tueren oeffenen gerade" wechseln.

				break;
			}
			case CABINDOOR_FULLY_CLOSED:
			case ENTRANCEDOOR_FULLY_CLOSED:
			{
				if(allDoorsClosedCurrentFloor()  && !m_descendingList.empty())                              //wenn alle Turen zu und absteigende Liste nicht leer
				{
					driveToFloor(m_descendingList.front());                                                 //fahre zum naechsten Stockwerk in der aufsteigenden Liste
					m_stateType= DESCENDING_DRIVING;                                                        //in den Zustand "Abwärtsfahrt, Fahrend" wechseln
				}

				if(allDoorsClosedCurrentFloor()  && m_descendingList.empty() && !m_ascendingList.empty())   //wenn alle Turen zu und absteigende Liste leer und aufsteigende Liste nicht leer
				{
					driveToFloor(m_ascendingList.front());                                                  //fahre zum naechsten Stockwerk in der aufsteigenden Liste
					m_stateType= ASCENDING_DRIVING;                                                         //in den Zustand "Aufwärtsfahrt, Fahrend" wechseln
				}

				if(allDoorsClosedCurrentFloor()  && m_descendingList.empty() && m_ascendingList.empty())    //wenn alle Turen zu und absteigende Liste leer und aufsteigende Liste leer
				{
					m_stateType= IDLE_DOORS_CLOSED;                                                         //in den Zustand "wartend, Tür geschlossen" wechseln
				}

				m_pCabinPanel->m_closeDoorsKeySignal =0;                                                    // LED Schließ-Taste zu

				break;
			}


			case FIREALERT:                                     // Feueralarm ausgeloest
			{

				m_pSystemController->pushEvent(ev);                                                         // fügt des Event den SystemKontroller hinzu
				openAllDoorsCurrentFloor();                                                                 // oeffne die alle Tuere
				m_stateType = FIREALARM_FINAL;                                                              // zustand wechseln
				m_pTimer->set(200);                                                                         // Timer setzen
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case FIREALARM_DRIVING: //wenn Zustand "Feueralarm ausgeloest, fahrend" ist
		{
			switch(ev.m_eventType)
			{

			case MOTOR_STOPPED:             //wenn Motor gestoppt
			{
				m_stateType = FIREALARM_FINAL;       //wechsel in den Zustand "Feueralarm_final"
				openAllDoorsCurrentFloor();         //oeffne alle Tueren
				m_pTimer->set(200);         //setze Timer auf halbe Sekunde
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}

		case FIREALARM_FINAL:
		{
			switch(ev.m_eventType)
			{

			case TIMER:
			{

				if(m_pCabinPanel->m_floorKeySignals[currentFloor()]){       //wenn Flurtastenbeleuchtung an
					for (unsigned int i=0; i< m_pEntrances.size(); i++){
						m_pCabinPanel->m_floorKeySignals[i] = 0;                                    //LED Taste Stockwerk aus
						m_pEntrances[i]->m_entrancePanel.m_upSignal= 0;                          //LED Taste "hoch" aus
						m_pEntrances[i]->m_entrancePanel.m_downSignal= 0;                            //LED Taste "runter" aus
						m_pCabinPanel->m_openDoorsKeySignal = 0;                        //LED Taste "oeffne Tueren" aus
						m_pCabinPanel->m_closeDoorsKeySignal = 0;                               //Led Taste "schliesse Tueren" aus
					}
					m_pTimer->set(200);                 //setze Timer
				}else{
					for (unsigned int i=0; i< m_pEntrances.size(); i++){
						m_pCabinPanel->m_floorKeySignals[i] = 1;                                    //LED Taste Stockwerk an
						m_pEntrances[i]->m_entrancePanel.m_upSignal= 1;                          //LED Taste "hoch" an
						m_pEntrances[i]->m_entrancePanel.m_downSignal= 1;                            //LED Taste "runter" an
						m_pCabinPanel->m_openDoorsKeySignal = 1;                            //LED Taste "oeffne Tueren" an
						m_pCabinPanel->m_closeDoorsKeySignal = 1;                       //Led Taste "schliesse Tueren" an
					}
					m_pTimer->set(200);             //setze Timer
				}
				break;
			}
			default:
			{
				break;
			}
			}
			break;
		}


		}
	}
}

/*! \fn CCabinController::CCabinController()
 *  \brief Konstruktor; Belegt alle Attribute mit Standardwerten (0, false), die Pointer mit 0.
 */
CCabinController::CCabinController()
{
	m_pMotor = 0;
	m_pTimer = 0;
	m_pHeightSensor = 0;
	m_pSystemController = 0;
	m_pCabinPanel = 0;
	m_targetFloor = 0;
	m_stateType = IDLE_DOORS_CLOSED;
}

/*! \fn CCabinController::connectSystemController(CSystemController* pSystemController)
 *  \brief Verbindet den Kabinencontroller mit dem Systemcontroller.
 *  \param pSystemController Pointer auf den Systemcontroller
 */
void CCabinController::connectSystemController(CSystemController* pSystemController)
{
	m_pSystemController = pSystemController;
}

/*! \fn CCabinController::connectHeightSensor(CHeightSensor* pHeightSensor)
 *  \brief Verbindet den Kabinencontroller mit dem Hoehensensor.
 *  \param pHeightSensor Pointer auf den Hoehensensor
 */
void CCabinController::connectHeightSensor(CHeightSensor* pHeightSensor)
{
	m_pHeightSensor = pHeightSensor;
}

/*! \fn CCabinController::connectMotor(CMotor* pMotor)
 *  \brief Verbindet den Kabinencontroller mit dem Motor.
 *  \param pMotor Pointer auf den Motor
 */
void CCabinController::connectMotor(CMotor* pMotor)
{
	m_pMotor=pMotor;
}

/*! \fn CCabinController::connectTimer(CTimer* pTimer)
 *  \brief Verbindet den Kabinencontroller mit dem Timer.
 *  \param pTimer Pointer auf den Timer
 */
void CCabinController::connectTimer(CTimer* pTimer)
{
	m_pTimer=pTimer;
	//m_pTimer->set(1);
}

/*! \fn CCabinController::driveToFloor(unsigned short destinationFloorNumber)
 *  \brief Veranlasst die Kabine, das angegebene Stockwerk anzufahren.
 *  \param destinationFloorNumber das Zielstockwerksnummer
 */
void CCabinController::driveToFloor(unsigned short destinationFloorNumber)
{
	float meters_diff = ((m_pHeightSensor->metersPerFloor())*(destinationFloorNumber)) - m_pHeightSensor->height();
	m_pMotor->lift(meters_diff);
	return;
}

/*! \fn CCabinController::currentFloor()
 *  \brief Liefert die der Kabine naechstgelegene Stockwerknummer zurueck.
 */
unsigned int CCabinController::currentFloor()
{
	return m_pHeightSensor->currentFloor();
}

/*! \fn CCabinController::pushEvent(SEvent event)
 *  \brief Fuegt dem Kabinencontroller ein Event hinzu.
 *  \param event Ereignis aus struct SEvent
 */
void CCabinController::pushEvent(SEvent event)
{
	printEvent(event, (void*) this);
	m_eventQueue.pushEvent(event);
}

/*! \fn CCabinController::connectCabinDoor(CCabinDoor* pCabinDoor)
 *  \brief Verbinder dem Kabinencontroller mit der Kabinentuer.
 *  \param pCabinDoor Pointer auf die Kabinentuer
 */
void CCabinController::connectCabinDoor(CCabinDoor* pCabinDoor)
{
	m_pCabinDoors[pCabinDoor->side()]=pCabinDoor;
}

/*! \fn CCabinController::connectCabinPanel(CCabinPanel* pCabinPanel)
 *  \brief Verbinder dem Kabinencontroller mit dem Kabinenpanel.
 *  \param pCabinPanel Pointer auf den Kabinenpanel
 */
void CCabinController::connectCabinPanel(CCabinPanel* pCabinPanel)
{
	m_pCabinPanel=pCabinPanel;
}

/*! \fn CCabinController::connectEntrance(CEntrance* pEntrance)
 *  \brief Verbindet dem Kabinencontroller mit der Zugang.
 *  \param pEntrance Pointer auf den Zugang
 */
void CCabinController::connectEntrance(CEntrance* pEntrance)
{
	m_pEntrances[pEntrance->m_floorNumber]=pEntrance;
}

/*! \fn CCabinController::openAllDoorsCurrentFloor()
 *  \brief oeffnet die Kabinentür(en) und die Zugangstür auf dem aktuellen Stockwerk
 */
void CCabinController::openAllDoorsCurrentFloor()
{
	m_pCabinDoors[m_pEntrances[currentFloor()]->m_entranceDoor.side()]->open();     // oeffne die Kabinentuer
	m_pEntrances[currentFloor()]->m_entranceDoor.open();                            // oeffne die Eingangstuer
}

/*! \fn CCabinController::closeAllDoorsCurrentFloor()
 *  \brief schliesst die Kabinentür(en) und die Zugangstür auf dem aktuellen Stockwerk
 */
void CCabinController::closeAllDoorsCurrentFloor()
{
	m_pCabinDoors[m_pEntrances[currentFloor()]->m_entranceDoor.side()]->close();        // schliesst die Kabinentuer
	m_pEntrances[currentFloor()]->m_entranceDoor.close();								// schliesst die Eingangstuer
}

/*! \fn CCabinController::addToAscendingList(unsigned int floor)
 *  \brief fuegt angegebenem Stockwerk in der aufsteigenden Liste hinzu, falls das angegebene
 *   Stockwerk noch in keiner der beiden Listen hinterlegt ist. Liste wird anschießend aufsteigend sortiert
 *  \param floor Stockwerksnummer
 */
void CCabinController::addToAscendingList(unsigned int floor)
{
	bool alreadyExists = true;
	unsigned int currentElement;

	std::list<unsigned int>::iterator it;


	for (it = m_ascendingList.begin(); it != m_ascendingList.end(); it++)
	{
		currentElement = *it;
		if (floor == currentElement)
		{
			alreadyExists = true;
			break;
		}
		else
			alreadyExists = false;
	}

	if (!alreadyExists)
	{

		for (it = m_descendingList.begin(); it != m_descendingList.end(); it++)
		{
			currentElement = *it;
			if (floor == currentElement)
			{
				alreadyExists = true;
				break;
			}
			else
				alreadyExists = false;
		}
	}

	if(!alreadyExists||m_ascendingList.empty())
	{
		m_ascendingList.push_back(floor);
		m_ascendingList.sort();
	}
}

/*! \fn CCabinController::addToDescendingList(unsigned int floor)
 *  \brief fuegt angegebenem Stockwerk in der absteigenden Liste hinzu, falls das angegebene
 *   Stockwerk noch in keiner der beiden Listen hinterlegt ist. Liste wird anschießend absteigend sortiert
 *  \param floor Stockwerksnummer
 */
void CCabinController::addToDescendingList(unsigned int floor)
{
	bool alreadyExists = true;
	unsigned int currentElement;

	std::list<unsigned int>::iterator it;


	for (it = m_descendingList.begin(); it != m_descendingList.end(); it++)
	{
		currentElement = *it;
		if (floor == currentElement)
		{
			alreadyExists = true;
			break;
		}
		else
			alreadyExists = false;
	}

	if (!alreadyExists)
	{

		for (it = m_ascendingList.begin(); it != m_ascendingList.end(); it++)
		{
			currentElement = *it;
			if (floor == currentElement)
			{
				alreadyExists = true;
				break;
			}
			else
				alreadyExists = false;
		}
	}

	if(!alreadyExists||m_descendingList.empty())
	{
		m_descendingList.push_back(floor);
		m_descendingList.sort();
		m_descendingList.reverse();
	}
}

/*! \fn CCabinController::allDoorsClosedCurrentFloor()
 *  \brief Liefert true zurueck, falls die Zugangstuer auf dem aktuellen Stockwerk und die Kabinentuer(en)
 *  geschlossen, sonst liefert false zurueck.
 */
bool CCabinController::allDoorsClosedCurrentFloor()
{
	if((m_pCabinDoors[m_pEntrances[currentFloor()]->m_entranceDoor.side()]->state()== CDoor::IS_CLOSED) && (m_pEntrances[currentFloor()]->m_entranceDoor.state() == CDoor::IS_CLOSED)) {
		return true;
	}
	else {
		return false;
	}
}

/*! \fn CCabinController::allDoorsOpenCurrentFloor()
 *  \brief Liefert true zurueck, falls die Zugangstuer auf dem aktuellen Stockwerk und die Kabinentuer(en)
 *   offen, sonst liefert false zurueck.
 */
bool CCabinController::allDoorsOpenCurrentFloor()
{
	if((m_pCabinDoors[m_pEntrances[currentFloor()]->m_entranceDoor.side()]->state()== CDoor::IS_OPEN) && (m_pEntrances[currentFloor()]->m_entranceDoor.state() == CDoor::IS_OPEN))
		return true;
	else
		return false;
}
