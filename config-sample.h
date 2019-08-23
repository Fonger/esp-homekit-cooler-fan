#define WIFI_SSID "wifi ap name"
#define WIFI_PASS "wifi password"

#define HOSTNAME "Fonger-Homekit"
#define HOMEKIT_PASSWORD "111-11-111"

#define DEFAULT_COOLER_TEMPERATURE 27
#define MIN_COOLER_TEMPERATURE 16
#define MAX_COOLER_TEMPERATURE 32

#define TEMPERATURE_POLL_PERIOD 10000
#define TEMPERATURE_SENSOR_GPIO 12 // D6
#define LED_GPIO 2                 // D4

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32

#define PROTOCOL SSD1306_PROTO_I2C
#define ADDR SSD1306_I2C_ADDR_0
#define I2C_BUS 0
#define SCL_PIN 4
#define SDA_PIN 5

#define DEFAULT_FONT FONT_FACE_TERMINUS_6X12_ISO8859_1

#define RELAY_GPIO 15 // D8
#define BTN_GPIO 3    // RX(high on boot)
