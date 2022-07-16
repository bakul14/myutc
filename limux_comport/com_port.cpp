//  C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <iterator>
#include <stdint.h>
#include <cstring>

//  Linux libraries
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()

using namespace std;

ifstream hex_file;

// Circuits to reprogramm:
const int precharge_circuit_id = 0x1;
const int discharge_circuit_id = 0x2;
const int vcu_id = 0x3;
const int bcu_id = 0x4;
const int ams_lv_id = 0x5;
const int brakelight_id = 0x6;
const int can_multi_front_id = 0x7;
const int can_multi_rear_id = 0x8;
const int ami_id = 0x9;
const int assi_id = 0xa;
const int fans_id = 0xb;
const int ams_master_id = 0xc;
const int ebs_id = 0xd;
const int steering_wheel_id = 0xe;
const int vcdu_id = 0xf;

int serial_port = 0;
struct termios tty;

int comport_init()
{
    serial_port = open("/dev/ttyACM1", O_RDWR);
    if (serial_port < 0)
    {
        cout << " Error open serial port\n\r";
        exit(1);
    }

    // Create new termios struct, we call it 'tty' for convention

    // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf(" Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf(" Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "incorrect arguments\n\r";
        return 1;
    }

    cout << " UTC-Bootloader\n\r";

    string str;
    if (comport_init() == 1) // init comport (only linux)
        cout << " Init COM port success\n\r";
    else
    {
        cout << " ERROR init COM port\n\r";
        exit(1);
    }

    hex_file.open(argv[1]);
    if (hex_file.fail())
    {
        cout << "FILE OPEN ERROR!\n\r";
        return 1;
    }
    cout << "hello\n\r";

    char data[8] = {'S', 'T', 'A', 'R', 'T', 'F', 'F', 'F'};
    char read_buf[44] = "0000000000000000000000000000000000000000000";
    char data_END[8] = {'E', 'N', 'D', 'F', 'F', 'F', 'F', 'F'};
    char data_tx_0[8] = {'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};
    char data_tx_1[8] = {'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};
    char data_tx_2[8] = {'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};
    char data_tx_3[8] = {'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};
    bool check_bytes = false;
    bool check_8_bytes = false;
    bool check_16_bytes = false;
    bool check_24_bytes = false;
    bool check_32_bytes = false;
    bool reading_over = false;
    int chosen_id = 0;
    int counter = 0;

    int line_counter = 0;
    int transmit_counter = 0;
    int feedback_counter = 0;

    cout << " Choose a circuit to reprogramm:\n\n\r"
            " 01---Precharge Circuit\r\n"
            " 02---Discharge Circuit\r\n"
            " 03---Vehicle Control Unit\r\n"
            " 04---Braking Control Unit\r\n"
            " 05---AMS LV \r\n"
            " 06---Brake Light\r\n"
            " 07---Can Multiplexer Front\r\n"
            " 08---Can Multiplexer Rear\r\n"
            " 09---Autonomous Missions Indicator\r\n"
            " 10---Autonomous System Status Indicator\r\n"
            " 11---Cooling Fans PCB\r\n"
            " 12---AMS Master\r\n"
            " 13---Emergency Brake System\r\n"
            " 14---Steering Wheel\r\n"
            " 15---Vehicle Control Dynamic Unit\r\n\n ";
    cin >> chosen_id;
    switch (chosen_id)
    {
    case 1:
        data[5] = '1';
        break;
    case 2:
        data[5] = '2';
        break;
    case 3:
        data[5] = '3';
        break;
    case 4:
        data[5] = '4';
        break;
    case 5:
        data[5] = '5';
        break;
    case 6:
        data[5] = '6';
        break;
    case 7:
        data[5] = '7';
        break;
    case 8:
        data[5] = '8';
        break;
    case 9:
        data[5] = '9';
        break;
    case 10:
        data[5] = 'A';
        break;
    case 11:
        data[5] = 'B';
        break;
    case 12:
        data[5] = 'C';
        break;
    case 13:
        data[5] = 'D';
        break;
    case 14:
        data[5] = 'E';
        break;
    case 15:
        data[5] = 'F';
        break;
    default:
        data[5] = '0';
    }

    cout << "\n\r Chosen circuit - " << data[5] << "\n\n\r";

    // Write to serial port
    printf("Sent %d\n\r", write(serial_port, data, sizeof(data)));
    while (1)
    {

        getline(hex_file, str);

        while (!hex_file.eof())
        {

            getline(hex_file, str);
            strcpy(read_buf, str.c_str());

            for (int i = 0; i <= 7; i++)
            {
                if (read_buf[1] == '1' && read_buf[2] == '0')
                {
                    check_bytes = true;
                    data_tx_0[i] = read_buf[i + 9];
                    data_tx_1[i] = read_buf[i + 17];
                    data_tx_2[i] = read_buf[i + 25];
                    data_tx_3[i] = read_buf[i + 33];
                }
                else
                {
                    check_bytes = true;
                    int last_position = 42;
                    int difference = 0;
                    while (read_buf[last_position] == '0' || read_buf[last_position] == '\0')
                        last_position--;
                    read_buf[1] = '1';
                    read_buf[2] = '0';
                    difference = 42 - last_position;
                    read_buf[41] = read_buf[last_position - 1];
                    read_buf[42] = read_buf[last_position];
                    for (int i = last_position - 1; i < 41; i++)
                        read_buf[i] = 'F';
                    data_tx_0[i] = read_buf[i + 9];
                    data_tx_1[i] = read_buf[i + 17];
                    data_tx_2[i] = read_buf[i + 25];
                    data_tx_3[i] = read_buf[i + 33];
                    reading_over = true;
                }
            }
            if (check_bytes == true)
            {
                line_counter++;
                if ((write(serial_port, data_tx_0, sizeof(data_tx_0))) != -1)
                    transmit_counter++;
                if ((write(serial_port, data_tx_1, sizeof(data_tx_1))) != -1)
                    transmit_counter++;
                if ((write(serial_port, data_tx_2, sizeof(data_tx_3))) != -1)
                    transmit_counter++;
                if ((write(serial_port, data_tx_3, sizeof(data_tx_3))) != -1)
                    transmit_counter++;
                if (reading_over == true)
                {
                    write(serial_port, data_END, sizeof(data_END));
                    cout << " Read----------" << line_counter << " lines\n\r"
                         << " Transmit------" << transmit_counter / 4 << " lines\n\n\r";
                    /////////////////////////////////////////////////////////////////////////////////////////////usb_rx
                    ///////////////////////////////////////////////////////////////////////////////////////////
                    // close(serial_port);
                    return 0;
                }
                for (int i2 = 0; i2 <= 7; i2++)
                {
                    data_tx_0[i2] = 'F';
                    data_tx_1[i2] = 'F';
                    data_tx_2[i2] = 'F';
                    data_tx_3[i2] = 'F';
                }
                for (int i = 0; i < 43; i++)
                    read_buf[i] = '0';
                read_buf[43] = '\0';
                check_bytes = false;
            }
        }
    }
    return 0;
}