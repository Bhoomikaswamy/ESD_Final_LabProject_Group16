# ESD_Final_LabProject_Group16
Access Control Using RFID

# Project Title 
RFID-Based Access Control System for Student Verification

# Background 
With the advancement of RFID technology, access control systems have become more efficient and secure. This project utilizes the RC522 RFID reader module interfaced with the TM4C123GH6PM microcontroller to create a system that reads student RFID tags and displays their details on a laptop via a serial interface like SSI, I2C, or UART. This system aims to facilitate smooth entry to educational institutions by automatically verifying student details.

# Problem Statement 
When an RFID tag is read by the RC522, the system should:
1. Retrieve and display the corresponding student details (student ID, name, branch, institute name) on the laptop using a serial communication interface.
2. Verify the branch and institute details against stored data.
3. Open the corresponding marks card file stored on the laptop if the verification is successful.

# Solution
1. Hardware Setup:
* Interface the RC522 RFID reader with the TM4C123GH6PM microcontroller using SPI, I2C, or UART.
* Connect the microcontroller to a computer via UART for data communication.
2. Software Development:
* Implement the RFID reading functionality.
* Use a database (or a simple text file) to store student details and marks cards.
* Write code to verify student details and open the marks card file on the laptop.
3. Communication:
* Establish communication between the TM4C123GH6PM and the laptop using the chosen interface.
* Display student details using a terminal emulator (e.g., PuTTY).

# Block Diagram
![RFID_Final_Block Diagram](https://github.com/user-attachments/assets/86652f42-3a4a-46d3-99b7-2402d4fe15f5)

# RFID(RC522 Interfacing) Diagram
![RC522_interface jpeg](https://github.com/user-attachments/assets/2e2b8ebb-b0e3-4f40-8b41-1524973b6f73)

# Pseudocode
This pseudocode outlines the main function of the RFID access control system:

```plaintext
function main() {
    // Step 1: Initialize system components
    initializeSystem()
    
    // Step 2: Start an infinite loop to monitor for RFID tags
    while true {
        // Step 3: Check if an RFID tag is detected
        if RFIDTagDetected() {
            // Step 4: Read data from the detected RFID tag
            studentData = readRFIDTag()
            
            // Step 5: Display the student data on the laptop screen
            displayOnLaptop(studentData)
            
            // Step 6: Verify the student's details
            if verifyStudentDetails(studentData) {
                // Step 7: If verification is successful, retrieve and display marks
                openMarksCard(studentData.id) // Open the marks card
                displayMarks(studentData.id)    // Display the student's marks
            } else {
                // Step 8: If verification fails, show an error message
                displayError("Verification Failed")
            }
        }
        
        // Step 9: Optional - Introduce a short delay to reduce CPU usage
        delay(100) // Delay for 100 milliseconds
    }
}

```
# Expected Outcome
The system will successfully read the RFID tag and display the student's information on the laptop.
It will verify the branch and institute details, and if they match, the corresponding marks card file will be opened and displayed.
In case of verification failure, an error message will be shown.
