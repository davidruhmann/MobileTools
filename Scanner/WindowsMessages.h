/* File:	WindowsMessages.h
 * Created:	
 * Author:	
 */

// Standard definitions used by framework and components

//
// Up to WM_USER + 32767 is allowed before possible conflicts occur.
//

// MMobile (0 - 140)
#define WM_BC_SCAN                        WM_USER + 1 // Barcode Scanner
#define WM_CONNECTIONMANAGER              WM_USER + 2
#define WM_UPDATE_ICON                    WM_USER + 3
#define WM_CONNECT_CHECK                  WM_USER + 4
// waitProgressDlg (6-12)
#define WM_RESIZE_VIEW                    WM_USER + 10
#define WM_MMOBILE_LOAD_COMPS             WM_USER + 11
#define WM_TIMEOUT_LOCK_APPLICATION	      WM_USER + 12 // Used to lock the application. WPARAM - CManualLogin::LOGIN_TYPE, LPARAM - 1 = modal dialogs are open
// 13 is for WM_CHECKBOX_STATUS_CHANGED
#define WM_TIMEOUT_LOCK_CHG_USER_CHECK    WM_USER + 13 // Used to check whether there are more tasks to perform after a change user from a timeout
#define WM_UPDATE_INIT_DOWNLOAD		      WM_USER + 14 // Used to setup the dialog for a file download. WPARAM - FileDownloadInitData pointer (must clean-up)
#define WM_UPDATE_CLOSE_DLG			      WM_USER + 15 // Used to close the update dialog after the downloading is complete. WPARAM - close code (IDOK, IDCANCEL)
#define WM_UPDATE_FILE_STATUS		      WM_USER + 16 // Used to update the current file download status. WPARAM - FileDownloadStatusData (must clean-up)
#define WM_UPDATE_PROCESS_DOWNLOAD_ERROR  WM_USER + 17 // Used when an error message needs to be displayed to a user about a filed download. WPARAM - CString* file name (must clean-up)
#define WM_UPDATE_ADD_TO_BYTE_COUNT       WM_USER + 18 // Used when the total byte count for the dialog needs to be updated. WPARAM - the number of bytes to add to the total count
#define WM_CARRIAGE_RETURN                WM_USER + 19 // Used when pressing enter at login/patient search screen			//010
#define WM_BC_SCAN2                       WM_USER + 20 // Barcode Scanner
#define WM_PLEASE_EXIT                    WM_USER + 21 // Used to exit app from MMORDERS
#define WM_PASSWORD_CHANGE                WM_USER + 22
#define WM_FORCE_EXIT                     WM_USER + 23 // Used to force exit app
#define WM_FINAL_SHUTDOWN                 WM_USER + 24 // Used to shutdown app via PostQuit
#define WM_SET_CONTROLS_TO_FRONT          WM_USER + 26 // Used pull controls to front after background set
#define WM_CHANGE_PROFILE                 WM_USER + 27 // Set when the profile changes on the login screen
#define WM_UPDATE_UI_INFO                 WM_USER + 28 // Set when data arrives from producer and is posted to the main thread
// 55 is used for WM_COMBO_STATE_MSG

// General Use (100 - 1990)
#define WM_CUSTOM_PATIENT_LIST_CANCEL_SELECTED			WM_USER + 100   // MMBizLogic, MMNursingServices
#define WM_CUSTOM_PATIENT_LIST_OK_SELECTED				WM_USER + 110   // MMBizLogic, MMNursingServices
#define WM_EDIT_LIST_SELECTIONS							WM_USER + 120   // MMBizLogic, MMNursingServices
#define GET_CONNECTIVITY_TIMER							WM_USER + 130   // MMCharge, MMRounding, MMSpecColl
#define WM_MORE_TEXT_TIMER								WM_USER + 140   // MMBizLogic, MMRounding
#define WM_MULTI_LOC_CANCEL_SELECTED					WM_USER + 150   // MMBizLogic, MMNursingServices
#define WM_MULTI_LOC_CONTINUE_SELECTED					WM_USER + 160   // MMBizLogic, MMNursingServices
#define WM_PATIENT_SELECTED								WM_USER + 170   // MMBizLogic, MMNursingServices
#define WM_PAT_LIST_SELECTED							WM_USER + 180   // MMCharge, MMPhysServices
#define WM_PERSONSEARCH_PERSON_SELECTED					WM_USER + 190   // MMBizLogic, MMOrders                           // Message to alert the parent window that a person has been selected
#define WM_PERSONSEARCH_PRSNL_SELECTED					WM_USER + 200   // MMBizLogic, MMOrders, MMPhysInbox, MMRxWriter  // Message to alert the parent window that a prsnl has been selected
#define WM_PERSONSEARCH_PAT_SELECTED					WM_USER + 210   // MMBizLogic, MMOrders, MMPhysInbox              // Message to alert the parent window that a patient has been selected
#define WM_PERSONSEARCH_PERSON_NONE						WM_USER + 220   // MMBizLogic, MMOrders, MMRxWriter               // Message to alert the parent window that a prsnl has been unselected
#define WM_PROVIDER_GROUP_PATIENT_LIST_CANCEL_SELECTED	WM_USER + 230   // MMBizLogic, MMNursingServices
#define WM_PROVIDER_GROUP_PATIENT_LIST_OK_SELECTED		WM_USER + 240   // MMBizLogic, MMNursingServices
#define WM_SHOW_MORE_TIMER								WM_USER + 250   // MMBizLogic, MMNursingServices
#define WM_VISIT_RELTN_CANCEL_SELECTED					WM_USER + 260   // MMBizLogic, MMNursingServices
#define WM_VISIT_RELTN_OK_SELECTED						WM_USER + 270   // MMBizLogic, MMNursingServices
#define WM_RXS_RM_SEL_CANCEL							WM_USER + 271   // MMNursingServices RxStation
#define WM_RXS_RM_SEL_CONTINUE							WM_USER + 272   // MMNursingServices RxStation
#define WM_RXS_RM_CT_SEL_BACK							WM_USER + 273   // MMNursingServices RxStation
#define WM_RXS_RM_CT_SEL_REMOVE							WM_USER + 274   // MMNursingServices RxStation
#define WM_REMOVE_COMPLETE								WM_USER + 275   // MMNursingServices RxStation
#define WM_DISPENSE_CONFIRMED							WM_USER + 276   // MMNursingServices RxStation
#define WM_DISPENSE_CONFIRMED_WASTE						WM_USER + 277   // MMNursingServices RxStation
#define WM_RXS_CHECK_QUANTITY							WM_USER + 278   // MMNursingServices RxStation
#define WM_COUNTBACK_OKAY								WM_USER + 279   // MMNursingServices RxStation
#define WM_RXS_BIN_STOP									WM_USER + 281   // MMNursingServices RxStation
#define WM_RXS_BIN_PRESENT								WM_USER + 282   // MMNursingServices RxStation
#define WM_RXS_BIN_NEXT									WM_USER + 283   // MMNursingServices RxStation
#define WM_RXS_WR_CLOSE									WM_USER + 284   // MMNursingServices RxStation
#define WM_RXS_WR_SEL_AMOUNT							WM_USER + 285   // MMNursingServices RxStation
#define WM_RXS_WRA_SEL_CANCEL							WM_USER + 286   // MMNursingServices RxStation
#define WM_RXS_WRA_SEL_COMPLETE							WM_USER + 287   // MMNursingServices RxStation
#define WM_RXS_RETURN_COMPLETE							WM_USER + 288   // MMNursingServices RxStation
#define WM_RXS_WR_DONE									WM_USER + 289   // MMNursingServices RxStation
#define WM_RXS_WRA_OTHER								WM_USER + 290   // MMNursingServices RxStation
#define WM_RXS_WRA_OTHER_COMPLETE						WM_USER + 291   // MMNursingServices RxStation
#define WM_POWERCHART_LIST_OK_SELECTED					WM_USER + 292   // MMBizLogic, MMNursingServices
#define WM_POWERCHART_LIST_CANCEL_SELECTED				WM_USER + 293   // MMBizLogic, MMNursingServices
#define WM_PATIENTS_SCANNED								WM_USER + 294   // MMNursingServices PPID
#define WM_PAT_LIST_REFRESH								WM_USER + 295   // MMNursingServices patient list
#define WM_CHECK_CHANGED								WM_USER + 296
#define WM_REQ_FIELD_SET								WM_USER + 297   // Message to send when a Required Field has been set
#define WM_REQ_FIELD_UNSET								WM_USER + 298   // Message to send when a Required Field has been Un-Set
#define WM_NO_SIZE_NO_DRAW_RESET						WM_USER + 299

#define WM_TREE_CLICKED									WM_USER + 301   // Used to notify a window that a tree has been clicked.

#define TODAYM_GETCOLOR					WM_USER + 100   // MMPhysInbox, MMRxWriter, MMUtils
#define CLC_REMOVED_ITEM				WM_USER + 290   // WCheckListCtrl
#define CLC_ITEM_CHECKED				WM_USER	+ 300   // WCheckListCtrl
#define WM_COMBO_STATE_MSG				WM_USER + 55    // WCombo
#define WM_WTREECTRL_ROW_SELECT			WM_USER + 320   // WTreeCtrl
#define WM_CANCEL_SYNC					WM_USER + 330   // WaitProgress
#define WM_END_OF_LIST					WM_USER + 674   // ColoredListCtrl, WListCtrl
#define WM_CHECK_BOX_STATUS_CHANGED		WM_USER + 13    // WCheckBox

// MMBizLogic (2000 - 2490)
#define WM_LOCATION_WAIT_TIMER			WM_USER + 2000
#define WM_RADIOLOGY_DOC_CLICKED		WM_USER	+ 2010
#define WM_PERSON_SEARCH_FINISHED		WM_USER + 2011
#define WM_PRSNL_SEARCH_FINISHED		WM_USER + 2012
#define WM_PATIENT_SEARCH_FINISHED		WM_USER + 2013
#define WM_RESULT_GRAPH_RESULT_CLICK	WM_USER + 2014  // WPARAM(LOWORD=x, HIWORD=y), LPARAM=ClinicalEvent* // 006
#define WM_ORG_SELECT_ORG_CHANGED		WM_USER + 2015  // LPARAM = Ptr to Organization that was selected
#define WM_PRSNL_SELECTED				WM_USER + 2016  // WPARAM = MMOBJLIB::PrsnlLite*

// MMCharge (2500 - 3490)
#define WM_ADD_CHARGE_CLOSE				WM_USER + 2500
#define WM_CHARGE_ENTRY_CLOSE			WM_USER + 2510
#define WM_CHARGE_ENTRY_COPY			WM_USER + 2520
#define WM_CHARGE_ENTRY_EDIT			WM_USER + 2530
#define WM_CHARGE_ENTRY_NEW				WM_USER + 2540
#define WM_NEW_FOLD						WM_USER + 2550
#define WM_SET_ACTION_BUTTON_STATES		WM_USER + 2560
#define WM_SUBMIT_TIMER					WM_USER + 2570

// MMDrugRef (3500 - 3990)
#define WM_HIDEALL						WM_USER + 3500
#define WM_RELOADINFO					WM_USER + 3510

// MMPhysInbox (4000 - 4990)
#define WM_BACK							WM_USER + 4000
#define WM_GO_AWAY						WM_USER + 4010
#define WM_HEADER_SUBMIT				WM_USER + 4020
#define WM_REFRESH						WM_USER + 4030
#define WM_REFRESH_ALARM				WM_USER + 4040
#define WM_RELOAD						WM_USER + 4050
#define WM_SELECT_ITEM					WM_USER + 4060

// MMRounding (5000 - 5990)
#define ONSIZE_TIMER					WM_USER + 5000

// MMRxWriter (6000 - 7990)
#define WM_ACCESSION_ENTERED			WM_USER + 6000
#define WM_BLACKHOLE					WM_USER + 6010
#define WM_CANCEL_PRESSED				WM_USER + 6020
#define WM_CONTINUE_PRESSED				WM_USER + 6030
#define WM_DRUG_CANCELED				WM_USER + 6040
#define WM_DRUG_SELECTED				WM_USER + 6050
#define FORMULARY_INFO_CLICK			WM_USER + 6060
#define WM_NEW_FOLDER					WM_USER + 6070
#define WM_PREF_DRUG_SELECTED			WM_USER + 6080
#define WM_SEARCH_BY_DRUG				WM_USER + 6090
#define WM_SEARCH_BY_PRODUCT			WM_USER + 6100
#define WM_SELECT_PRESSED				WM_USER + 6110
#define WM_UPDATE_MEDPROFILE			WM_USER + 6120

// MMSpecColl (8000 - 9990)
#define WM_ACCESSION_ADDED				WM_USER + 8000 
#define WM_ACCESSION_NOT_ADDED			WM_USER + 8010
#define WM_ACCESSION_SCANNED			WM_USER + 8020
#define WM_ADDING_COMMENTS				WM_USER + 8030
#define WM_AD_HOC_ACC					WM_USER + 8040
#define WM_AD_HOC_PAT					WM_USER + 8050
#define WM_BC_PASS						WM_USER + 8060
#define WM_CLOSE_DLG					WM_USER + 8070
#define WM_COMMENT_HIDE					WM_USER + 8080
#define WM_DONE_WITH_COMMENTS			WM_USER + 8090
#define WM_LIST_SELECTED				WM_USER + 8100
#define WM_LOAD_PREFS					WM_USER + 8110
#define WM_METHOD_MODIFIED				WM_USER + 8120
#define WM_NO_AD_HOC_ACC				WM_USER + 8130
#define WM_NO_AD_HOC_PAT				WM_USER + 8140
#define WM_NO_METHOD_MODIFIED			WM_USER + 8150
#define WM_ORG_CHANGED					WM_USER + 8160
#define WM_OVERRIDE						WM_USER + 8170
#define WM_OVERRIDE_NO					WM_USER + 8180
#define WM_UNKNOWN_ENTERED				WM_USER + 8190
#define WM_UPDATE_CONNECTION			WM_USER + 8200

// MMOrders (10000 - 12990)
#define WM_ADHOCMED_ALL_REQ_VALUES_SET         WM_USER + 10010  // Sent when required fields are set
#define WM_ADHOCMED_MAXIMIZE                   WM_USER + 10020  // Sent when the maximize button is clicked
#define WM_ADHOCMED_MINIMIZE                   WM_USER + 10030  // Sent when the minimize button is clicked
#define WM_ADHOCMED_PRODUCT_IN_LIST            WM_USER + 10040  // Sent when the first product is populated into the list
#define WM_ADHOCMED_PRODUCT_SELECTED           WM_USER + 10050  // For when a product a selected
#define WM_ADHOCMED_REQ_VALUES_NOT_SET         WM_USER + 10060  // Sent when required fields are not set
#define WM_ADHOCMED_DOSE_CHANGED               WM_USER + 10061  // Sent whenever the dose is changed
#define WM_ARE_REQUIRED_FIELDS_SET             WM_USER + 10070
#define WM_DATE_SAVE                           WM_USER + 10090
#define WM_SELECT_ASSOCIATE_ORDER_CHANNEL      WM_USER + 10091
#define WM_SELECT_REFRESH_DEVICE_ASSOCIATIONS  WM_USER + 10092
#define WM_DISASSOCIATE_FROM_MED_DOC           WM_USER + 10093
#define	WM_DILUENT_CHANGE                      WM_USER + 10100  // Used to signify that the diluent has changed
#define WM_DO_OTHER                            WM_USER + 10110
#define WM_DO_WITNESS                          WM_USER + 10120
#define WM_ENABLE_BUTTONS                      WM_USER + 10130  // Used to pass back whether to enable sign/cancel buttons
#define WM_FILTER_ALARM                        WM_USER + 10140
#define WM_INGREDIENTS_CHOSEN                  WM_USER + 10150  // Used to signify that any ingredients have been chosen
#define WM_INGREDIENTS_NONE                    WM_USER + 10160  // Used to signify that no ingredients have been chosen	
#define WM_INGREDIENT_WITNESS                  WM_USER + 10170  // Used to signify that witnessing is needed based on an ingredient
#define WM_INGREDIENT_NO_EDIT                  WM_USER + 10180  // Used to signify that the dialog isn't alterable
#define WM_MED_DOSE_CHANGE                     WM_USER + 10186  // Meds Documentation, alerts a parent that a child's dose has been changed
#define WM_MED_DOSE_VOLUME_CHANGE              WM_USER + 10189  // Meds Documentation, when a med volume has been changed (if dose is a volume, dose change still sent)
#define WM_MED_ROUTE_CHANGE                    WM_USER + 10187  // Meds documentation, alerts a parent that the route has been set / altered
#define WM_MED_UPDATE_SIGN_BUTTON              WM_USER + 10188  // Meds Documentation, used to tell when the sign button status needs to get updated
#define WM_NDC_BARCODE_SCAN                    WM_USER + 10190  // Meds Documentation
#define WM_MED_IV_PIGGYBACK                    WM_USER + 10191  // Meds Documentation, alerts a parent to launch the IV Piggyback workflow (if possible)
#define WM_OTHER_DONE                          WM_USER + 10200
#define WM_POPUP                               WM_USER + 10210
#define WM_POP_KEY                             WM_USER + 10220
#define WM_POWERCONTROL_FOCUS                  WM_USER + 10230  // PowerControl has focus
#define WM_POWERFORMS_LITE_FOCUS               WM_USER + 10240
#define WM_POWERFORMS_FIELD_SET                WM_USER + 10250
#define WM_POWERFORMS_REQ_FIELDS_SET           WM_USER + 10260
#define WM_POWERFORMS_REQ_FIELDS_NOT_SET       WM_USER + 10270
#define WM_POWERFORMS_REQ_FIELD_UNSET          WM_USER + 10280  // Message to send when a Required Field has been Un-Set
#define WM_POWERFORMS_SUBMIT_ON_SAVE           WM_USER + 10290
#define WM_POWERFORMS_LABEL_CLICK              WM_USER + 10291  // WPARAM & LPARAM ignored // 005
#define WM_RESULT_TREND_RESULT_SELECTED        WM_USER + 10292  // WPARAM = ClinicalEvent* & LPARAM ignored // 005
#define WM_PRODUCT_REQUIRES_WITNESS            WM_USER + 10300  // Sent if the product requires a witness
#define WM_PRODUCT_REQUIRES_NO_WITNESS         WM_USER + 10310  // Sent if the product does not require a witness
#define	WM_REQUIRED_INGREDIENT_NOT_SET         WM_USER + 10320  // Used to signify that required ingredients are not all set
#define	WM_REQUIRED_INGREDIENT_SET             WM_USER + 10330  // Used to signify that required ingredients are all set
#define WM_RESIZE_EVENT_FORM                   WM_USER + 10340
#define WM_RESIZE_FORM                         WM_USER + 10350
#define	WM_RESIZE_INGREDIENT_SELECTION         WM_USER + 10360  // Used to signify that the ingredient selection dialog has changed size
#define WM_SAVE_SIGN                           WM_USER + 10370
#define WM_SCAN_BEFORE_SIGN_RESET              WM_USER + 10380
#define WM_SHOW_DELTA                          WM_USER + 10390
#define WM_SHOW_WEIGHT                         WM_USER + 10400
#define WM_SIZEITONCE                          WM_USER + 10420
#define WM_ADHOCMED_DOSE_VOL_CHANGE            WM_USER + 10421
#define WM_NOTIFY_PARENT                       WM_USER + 10430  // Used to tell when to start/stop notifying the parent of a window.  WPARAM: BOOL - Whether to start notifying or stop
#define WM_SELECT_ALLOWED                      WM_USER + 10431  // Asks if the selection is allowed 
#define WM_CONNECT_TO_DEVICE                   WM_USER + 10450  // Used to notify a window to connect to a desired device.
#define WM_IV_INGREDIENT_CHANGE                WM_USER + 10460  // Used to notify a window that the IV Ingredient amounts have changed
#define WM_IV_MANUAL_CHANGE_FLAG               WM_USER + 10470  // Used to do post messages to alter the value used to determine if changes are being made manually
                                                                //  WPARAM: BOOL - Whether any changes happening now are being done manually by the user
#define WM_IV_PUMP_STATUS_CHANGE               WM_USER + 10471  // Used to notify a window that the IV Pump status has changed
#define WM_DOING_NURSE_COLLECT                 WM_USER + 10472
#define WM_NOT_DOING_NURSE_COLLECT             WM_USER + 10473
#define WM_IV_RETRO_DATE_SET                   WM_USER + 10474  // Used to tell when an IV has had a retrospective date set on it.
