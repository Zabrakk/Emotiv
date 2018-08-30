#include <iostream>
#include <string>
#include <windows.h>

#include "Iedk.h"
#include "IEmoStateDLL.h"
#include "IedkErrorCode.h"

using namespace std;

string profileForLoading = "profile.emu";
bool continueLoop = true;
unsigned long actionList = 0;

void trainMC(int ID);

void ldProfile(int ID) {
	if (IEE_LoadUserProfile(ID, profileForLoading.c_str()) == EDK_OK) {
		cout << "Profile loaded!" << endl;
	}
	else {
		cout << "Can't load profile!" << endl;
	}
}

void saveProfile(int ID) {
	if (IEE_SaveUserProfile(ID, profileForLoading.c_str()) == EDK_OK) {
		cout << "Profile saved!" << endl;
	}
	else {
		cout << "Saving failed!" << endl;
	}
}

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
		continueLoop = false;
	}
}

void handleMentalCommand(EmoStateHandle eState, int ID) {
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

void setActiveMC(int ID) {
	unsigned long action1 = (unsigned long)IEE_MentalCommandAction_t::MC_PUSH;
	unsigned long action2 = (unsigned long)IEE_MentalCommandAction_t::MC_LIFT;
	unsigned long action3 = (unsigned long)IEE_MentalCommandAction_t::MC_PULL;
	actionList = action1 | action2 | action3;

	int code = IEE_MentalCommandSetActiveActions(ID, actionList);
	if (code == EDK_OK) {
		cout << "Mental commands PUSH, PULL and LIFT set active for user " << ID << endl;
	}
	else {
		cout << "Error setting actions active" << endl;
	}
}

void setMCAction(int ID, IEE_MentalCommandAction_t action) {
	int code = IEE_MentalCommandSetTrainingAction(ID, action);
	code = IEE_MentalCommandSetTrainingControl(ID, MC_START);

	switch (action) {
	case MC_PUSH: { cout << "Training push" << endl; break; }
	case MC_LIFT: { cout << "Training lift" << endl; break; }
	case MC_PULL: { cout << "Training pull" << endl; break; }
	}
}

void handleMentalCommandEvent(EmoEngineEventHandle eEvent, int ID) {
	IEE_MentalCommandEvent_t eventType = IEE_MentalCommandEventGetType(eEvent);
	int opt = 0;

	if (eventType == IEE_MentalCommandTrainingStarted) {
		cout << "Training started" << endl;
	}
	else if (eventType == IEE_MentalCommandTrainingSucceeded) {
		cout << "Training succeedes" << endl;
		cout << "1. Accept training, 2. Reject training ";
		cin >> opt;
		if (opt == 1) {
			IEE_MentalCommandSetTrainingControl(ID, MC_ACCEPT);
		}
		else {
			IEE_MentalCommandSetTrainingControl(ID, MC_REJECT);
		}
	}
	else if (eventType == IEE_MentalCommandTrainingFailed) {
		cout << "Training failed" << endl;
		trainMC(ID);
	} 
	else if (eventType == IEE_MentalCommandTrainingRejected) {
		cout << "Training rejected by user " << ID << endl;
		trainMC(ID);
	}
	else if (eventType == IEE_MentalCommandTrainingCompleted) {
		cout << "Training completed" << endl;
		cout << "1. Save progress, Other Don't save ";
		cin >> opt;
		if (opt == 1) {
			saveProfile(ID);
		}
		trainMC(ID);
	}
}


void trainMC(int ID) {
	int opt = 0;
	cout << "Training Mental Commands" << endl;
	cout << "Enter: 1. Push, 2. Lift, 3. Pull, other for exit ";
	cin >> opt;

	if (opt == 1) {
		setMCAction(ID, MC_PUSH);
	}
	else if (opt == 2) {
		setMCAction(ID, MC_LIFT);
	}
	else if (opt == 3) {
		setMCAction(ID, MC_PULL);
	}
}

int main() {
	const char Host[] = "127.0.0.1";
	EmoEngineEventHandle eEvent = IEE_EmoEngineEventCreate();
	EmoStateHandle eState = IEE_EmoStateCreate();
	const int COMPOSER_PORT = 1726;
	unsigned long userID = 0;
	int option = 0;
	int option2 = 0;

	//Connecting to EmoEngine
	cout << "1: Composer, 2: Headset ";
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
	cout << "--------------------------------------" << endl;
	cout << "1. for loading profile, other for training commands ";
	cin >> option2;
	while (continueLoop) {
		int state = IEE_EngineGetNextEvent(eEvent);
		if (state == EDK_OK) {
			IEE_Event_t eventType = IEE_EmoEngineEventGetType(eEvent);
			IEE_EmoEngineEventGetUserId(eEvent, &userID);
			if (eventType == IEE_UserAdded) {
				cout << "New user " << userID << " added" << endl;

				if (option2 == 1) {
					ldProfile(userID);
				}
				setActiveMC(userID);
				trainMC(userID);
			} else if (eventType == IEE_EmoStateUpdated) {
				handleExpression(eEvent, eState);
				handleMentalCommand(eState, userID);
			} else if (eventType == IEE_MentalCommandEvent) {
				handleMentalCommandEvent(eEvent, userID);
			}
		}
	}

	IEE_EngineDisconnect();
	IEE_EmoStateFree(eState);
	IEE_EmoEngineEventFree(eEvent);

	system("pause");
	return 0;

}
