#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define WR_RGB_VALUE _IOW('a', 1, struct rgb_value)
#define RD_RGB_VALUE _IOR('a', 2, struct rgb_value)
#define WR_LCD_TEXT _IOW('a', 3, char *)
#define RD_LCD_TEXT _IOR('a', 4, char *)

struct rgb_value {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

int main() {
    int fd;
    struct rgb_value rgb_data, read_data;
    char lcd_text[256];
    char read_lcd_text[256];
    int option;

    printf("Opening Driver...\n");
    fd = open("/dev/rgb_lcd", O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }

    while (1) {
        printf("\nChoose an operation:\n");
        printf("1. Read RGB values\n");
        printf("2. Write RGB values\n");
        printf("3. Read LCD text value\n");
        printf("4. Write LCD text value\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                // Read RGB values from driver
                printf("Reading RGB Values from Driver...\n");
                ioctl(fd, RD_RGB_VALUE, (struct rgb_value*) &read_data);
                printf("RGB Values are R: %d, G: %d, B: %d\n", read_data.red, read_data.green, read_data.blue);
                break;
            case 2:
                // Write RGB values to driver
                printf("Enter the RGB values (0-255) to send: \n");
                printf("Red: ");
                scanf("%hhu", &rgb_data.red);
                printf("Green: ");
                scanf("%hhu", &rgb_data.green);
                printf("Blue: ");
                scanf("%hhu", &rgb_data.blue);

                // Print RGB values before writing to driver
                printf("RGB Values to write: R: %d, G: %d, B: %d\n", rgb_data.red, rgb_data.green, rgb_data.blue);

                printf("Writing RGB Values to Driver...\n");
                ioctl(fd, WR_RGB_VALUE, (struct rgb_value*) &rgb_data);
                break;
            case 3:
                // Read LCD text from driver
                printf("Reading LCD Text from Driver...\n");
                ioctl(fd, RD_LCD_TEXT, read_lcd_text);
                printf("LCD Text is: %s\n", read_lcd_text);
                break;
            case 4:
                // Get LCD text from user
                printf("Enter the LCD text to send: \n");
                getchar(); // Clear newline character left by previous scanf
                fgets(lcd_text, sizeof(lcd_text), stdin);
                lcd_text[strcspn(lcd_text, "\n")] = 0; // Remove newline character

                // Print LCD text before writing to driver
                printf("LCD Text to write: %s\n", lcd_text);

                // Write LCD text to driver
                printf("Writing LCD Text to Driver...\n");
                ioctl(fd, WR_LCD_TEXT, lcd_text);
                break;
            case 5:
                printf("Exiting...\n");
                close(fd);
                return 0;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }

    return 0;
}

