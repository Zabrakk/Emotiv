import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Scanner;
import com.emotiv.Iedk.*;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.*;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

//Requires Emotiv SDK and the RXTX library (http://rxtx.qbang.org/wiki/index.php/Download)

/*
 * Facial expressions include Upper: Surprise, (Frown not used anywhere in API)
 * Eye related: Blink, wink right/left
 * Lower: Smile, clench, (laugh, not used in API)
 */


public class Facial_Expression implements SerialPortEventListener {
	SerialPort port;
	private static final String PORT_NAMES[] = { "COM4", "COM10"}; //Change this to the port shown on Arduino IDE 
	private OutputStream output; //Output for the port
	private BufferedReader input; //Input (not used)
	private static final int DATA_RATE = 9600;
	
	public void initializeArduinoConn() {
		System.out.println("Connecting to Arduino");
		CommPortIdentifier portId = null;
		Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();
		//Instance of serial port
		while (portEnum.hasMoreElements()) {
			CommPortIdentifier currPortId = (CommPortIdentifier) portEnum.nextElement();
			for (String portName : PORT_NAMES) {
				if (currPortId.getName().equals(portName)) {
					portId = currPortId;
					break;
				}
			}
		}
		
		if (portId == null) {
			System.out.println("Could not find COM port.");
			System.out.println("Could not connect to Arduino");
			System.exit(0); //Close program if we can't connect to Arduino
		}
		
		try { 
			port = (SerialPort) portId.open(this.getClass().getName(), 2000); //Open serial port
			port.setSerialPortParams(DATA_RATE, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE); //Set port params
			output = port.getOutputStream(); //Open the outputsream
			input = new BufferedReader(new InputStreamReader(port.getInputStream()));
			//Event listeners
			port.addEventListener(this);
			port.notifyOnDataAvailable(true);
		} catch (Exception e) {
			System.out.println(e.toString());
		}
	}
	
	//This will be called when we stop using the port
	public synchronized void close() {
		if (port != null) {
			port.removeEventListener();
			port.close();
		}
	}
	
	@Override
	public void serialEvent(SerialPortEvent event) {
		if (event.getEventType() == SerialPortEvent.DATA_AVAILABLE) {
			try {
				String inputLine=input.readLine();
				System.out.println(inputLine);
			} catch (Exception e) {
				System.err.println(e.toString());
			}
		}
	} 
	
	public static void handleExpression(Pointer eEvent, Pointer eState, Facial_Expression main) {
		try {
			Edk.INSTANCE.IEE_EmoEngineEventGetEmoState(eEvent, eState); // Copies an EmoState returned with a IEE_EmoStateUpdate event to memory
			//Test for different facial events after this command ^
			
			if(EmoState.INSTANCE.IS_FacialExpressionIsBlink(eState) == 1) {
				System.out.println("Blink");
				main.output.write("1".getBytes());
			} else if(EmoState.INSTANCE.IS_FacialExpressionIsRightWink(eState) == 1) {
				System.out.println("Right Wink");
				main.output.write("2".getBytes());
			} else if(EmoState.INSTANCE.IS_FacialExpressionIsLeftWink(eState) == 1) {
				System.out.println("Left Wink");
				main.output.write("3".getBytes());
			} else if(EmoState.INSTANCE.IS_FacialExpressionGetSmileExtent(eState) >= 0.6) {
				System.out.println("Smile");
				main.output.write("4".getBytes());
			} else if(EmoState.INSTANCE.IS_FacialExpressionGetClenchExtent(eState) >= 0.6) {
				System.out.println("Clench, closing program");
				System.out.println("Disconnecting from engine and freeing up memory");
				Edk.INSTANCE.IEE_EngineDisconnect(); //This has to be called before closing program
				Edk.INSTANCE.IEE_EmoStateFree(eState); //Free up memory
				Edk.INSTANCE.IEE_EmoEngineEventFree(eEvent);
			} else {
				System.out.println("Expression not recognized");
			}
		} catch (Exception e) { }
	}
	
	public static void main(String[] args) {
		//Creation of core objects. IEE_ functions modify or retrieve EmoEngine settings
		Pointer eEvent = Edk.INSTANCE.IEE_EmoEngineEventCreate(); //Holds an EmoEngine event, returns EmoEngineEventHandle
		Pointer eState = Edk.INSTANCE.IEE_EmoStateCreate(); //Stores an EmoState, returns EmoStateHandle
		IntByReference userID = null; //User ID
		Scanner scan = new Scanner(System.in);
		int option;
		
		System.out.println("Choose suitable option: 1. Using the Emotiv headset, 2. Using Xavier Composer");
		while (true) {
			try {
				option = scan.nextInt();
				if (option == 1 ) {
					//Trying to connect to the headset without it being connected seem to not fail but obviosly this does not work					
					System.out.println("Connecting to Headset");
					if (Edk.INSTANCE.IEE_EngineConnect("Emotiv Systems-5") != EdkErrorCode.EDK_OK.ToInt()) {
						System.out.println("Emotiv Engine start up failed.");
						Edk.INSTANCE.IEE_EmoStateFree(eState); //Free up memory
						Edk.INSTANCE.IEE_EmoEngineEventFree(eEvent);
						return;
					} else {
						break;
					}
				} else if (option == 2) {
					System.out.println("Connecting to Xavier Composer");
					if (Edk.INSTANCE.IEE_EngineRemoteConnect("127.0.0.1", (short) 1726, "Emotiv Systems-5") != EdkErrorCode.EDK_OK.ToInt()) {
						//Seems like strDevID can be anything
						System.out.println("Emotiv Engine start up failed.");
						Edk.INSTANCE.IEE_EmoStateFree(eState);
						Edk.INSTANCE.IEE_EmoEngineEventFree(eEvent);
						return;
					} else {
						break;
					}
				} else {
					System.out.println("Invalid option");
				}
			} catch(Exception e) {
				System.out.println("Input a number");
			}
		}
		
		Facial_Expression main = new Facial_Expression();
		main.initializeArduinoConn();
		Thread thread = new Thread() {
			int state = 0;
			public void run() {
					try {
						System.out.println("Connection succesfull");
						while(true) { //Loop fot tracking events
							state = Edk.INSTANCE.IEE_EngineGetNextEvent(eEvent); //Retrives the next EmoEngine event
							if (state == EdkErrorCode.EDK_OK.ToInt()) { //New event needs to be handeled
								//System.out.println("State OK");
								int eventType = Edk.INSTANCE.IEE_EmoEngineEventGetType(eEvent); //Returns the type of the event retrieved by NextEvent()
								//Edk.INSTANCE.IEE_EmoEngineEventGetUserId(eEvent, userID);
								if(eventType == Edk.IEE_Event_t.IEE_EmoStateUpdated.ToInt()){ //Check if EmoState udated
									//New EmoState from user
									handleExpression(eEvent, eState, main);
									
								} else {
									System.out.println("EmoState didn't update");
								}
							}
						}
					} catch (Exception e) {	}
				}
		};
		thread.start();
	}
	
}
