#include <iostream>
#include <string>
#include <windows.h>

#include "Iedk.h"
#include "IEmoStateDLL.h"
#include "IedkErrorCode.h"

using namespace std;


void handleExpression(EmoEngineEventHandle eEvent, EmoStateHandle eState) {
	IEE_EmoEngineEventGetEmoState(eEvent, eState);
	if (IS_FacialExpressionIsBlink(eState)) {
		cout << "Blink" << endl;
	}
	else if (IS_FacialExpressionIsRightWink(eState)) {
		cout << "Right wink" << endl;
	}
	else if (IS_FacialExpressionIsLeftWink(eState)) {
		cout << "Left wink" << endl;
	}
	else if (IS_FacialExpressionGetSmileExtent(eState) >= 0.6) {
		cout << "Smile" << endl;
	}
	else if (IS_FacialExpressionGetClenchExtent(eState) >= 0.6) {
		cout << "Clench" << endl;
	}
}

void handleMentalCommand(EmoStateHandle eState) {
	IEE_MentalCommandAction_t actionType = IS_MentalCommandGetCurrentAction(eState);
	float power = IS_MentalCommandGetCurrentActionPower(eState);
	if (actionType != 1) {
		if (actionType == MC_PUSH && power >= 0.6) {
			cout << "Push" << endl;
		}
		else if (actionType == MC_PULL && power >= 0.6) {
			cout << "Pull" << endl;
		}
		else if (actionType == MC_LIFT && power >= 0.6) {
			cout << "Lift" << endl;
		}
		else if (actionType == MC_DROP && power >= 0.6) {
			cout << "Drop" << endl;
		}
	}
}

int main() {
	const char Host[] = "127.0.0.1";
	EmoEngineEventHandle eEvent = IEE_EmoEngineEventCreate();
	EmoStateHandle eState = IEE_EmoStateCreate();
	unsigned int userID = 0;
	const int COMPOSER_PORT = 1726;
	int option = 0;

	//Connecting to EmoEngine
	cout << "1: Composer, 2: Headset" << endl;
	while (true) {
		cin >> option;
		if (option == 1) {
			if (IEE_EngineRemoteConnect(Host, COMPOSER_PORT) != EDK_OK) {
				cout << "Connection to Composer failed" << endl;
				exit(1);
			}
			break;
		}
		else if (option == 2) { //This never fails but it doesn't actually connect if there is no headset
			if (IEE_EngineConnect() != EDK_OK) {
				cout << "Connection ro headset failed" << endl;
				exit(1);
			}
			break;
		}
	}

	//Handeling input
	cout << "Connection succesful" << endl;
	while (true) {
		int state = IEE_EngineGetNextEvent(eEvent);
		if (state == EDK_OK) {
			IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
			IEE_EmoEngineEventGetUserId(eEvent, &userID);
			if (eventType == IEE_UserAdded) {
				cout << "New user " << userID << " added" << endl;
			} else if (eventType == IEE_EmoStateUpdated) {
				handleExpression(eEvent, eState);
				handleMentalCommand(eState);
			} else if (eventType == IEE_MentalCommandEvent) {
				cout << "Mental command" << endl;
			}
		}
	}

	system("pause");
	return 0;

}