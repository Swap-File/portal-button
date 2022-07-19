// expose variables?
// add custom external message.
// scrolling custom message
// override all fixed message?
// handle a message changing mid scroll!

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Particle.h"

SerialLogHandler logHandler;
Adafruit_SSD1306 display(2);

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

uint32_t ranking = 0;

#define MESSAGE_LIMIT_ACTIVE 4
#define MESSAGE_LIMIT_IDLE 1

#define PUBLISH_TIME 5000 // expose this?
#define BLUE_LED_BLINK_RATE 100

#define PHY_USER_TIMEOUT 60000 // expose this?
#define WEB_USER_TIMEOUT 60000 // expose this?

// actual counts
volatile unsigned int phy_count;
unsigned int web_count;

// how many people are using the button
unsigned int web_users = 0;
unsigned int phy_users = 0;

// the todo list from the web
unsigned int web_count_todo = 0;
unsigned int web_rate_todo = 0;

// keeps track of if the red LED is turned off due to physical or web button presses
bool web_pressed = false;
bool phy_pressed = false;

// activity check
static bool phy_active = false;
static bool web_active = false;
static system_tick_t web_count_time = -WEB_USER_TIMEOUT;   // set when web_count_todo is done
volatile system_tick_t phy_count_time = -PHY_USER_TIMEOUT; // set in ISR

int batterylevel = 0;
int signal_rssi = 0;

// user count
// ranking
const char string_display_url[] = "www.portalbutton.com";
char temp_string[50];

char count_text[15]; // scratch space to store string
static bool flip_display = false;

char web_text[128] = {0};

const char string_0[] = "If you become light-headed from thirst, feel free to pass out.";
const char string_1[] = "Any appearance of danger is merely a device to enhance your testing experience.";
const char string_2[] = "When the testing is over, you will be missed.";
const char string_3[] = "Cake and grief counseling will be available at the conclusion of the test.";
const char string_4[] = "Thank you for helping us help you help us all.";
const char string_5[] = "The experiment is nearing its conclusion.";
const char string_6[] = "Button-based testing remains an important tool for science.";
const char string_7[] = "For your testing convenience, all button related safety precautions have been deactivated.";
const char string_8[] = "The Enrichment Center reminds you that the Button will never threaten to stab you.";
const char string_9[] = "Prolonged exposure to the Button is not part of this test.";
const char string_10[] = "Thanks to Emergency Testing Protocols, testing can continue.";
const char string_11[] = "If you feel liquid running down your neck, relax, lie on your back, and apply immediate pressure to your temples.";
const char string_12[] = "Because of the technical difficulties we are currently experiencing, your test environment is unsupervised.";
const char string_13[] = "Some emergency testing may require prolonged interaction with lethal military androids.";
const char string_14[] = "Testing is the future, and the future starts with you.";
const char string_15[] = "To ensure that sufficient power remains for core testing protocols, all safety devices have been disabled.";
const char string_16[] = "For your privacy, test results are not being relayed via satelite to long term storage for future review.";

const char *const string_table[] = {
    string_0,
    string_1,
    string_2,
    string_3,
    string_4,
    string_5,
    string_6,
    string_7,
    string_8,
    string_9,
    string_10,
    string_11,
    string_12,
    string_13,
    string_14,
    string_15,
    string_16};

void button_press(void)
{
  phy_count++;
  phy_count_time = millis();
  ranking += 1000;
}

// 4ms total time to reinit
void display_reinit(void)
{

  if (batterylevel < 14)
    display.reset(1);
  else
    display.reset(0);

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(1);
  display.setTextWrap(false);
}

void blue_led_handler(uint8_t request)
{
  static int blue_blinks_todo = -1;
  static system_tick_t blue_blink_time = 0;

  blue_blinks_todo += (request << 1);

  if (blue_blinks_todo >= 0)
  {
    if (millis() - blue_blink_time > BLUE_LED_BLINK_RATE)
    {
      (blue_blinks_todo-- % 2) ? digitalWriteFast(D7, HIGH) : digitalWriteFast(D7, LOW);
      blue_blink_time = millis();
    }
  }
}

void red_led_handler(void)
{
  phy_pressed = digitalRead(D2) ? false : true;
  (phy_pressed || web_pressed) ? digitalWriteFast(D4, HIGH) : digitalWriteFast(D4, LOW);
}

void eeprom_load(void)
{
  unsigned int phy_count_temp;
  EEPROM.get(0, phy_count_temp);
  phy_count = phy_count_temp;
  EEPROM.get(4, web_count);
}

// 100ms worst case
bool eeprom_store_web(void)
{
  static unsigned int web_count_saved = 0;
  if (web_count_saved != web_count)
  {
    web_count_saved = web_count;
    EEPROM.put(4, web_count);
    return true;
  }
  return false;
}

// 100ms worst case
bool eeprom_store_phy(void)
{
  unsigned int phy_count_temp = phy_count;
  static unsigned int phy_count_saved = 0;
  if (phy_count_saved != phy_count_temp)
  {
    phy_count_saved = phy_count_temp;
    EEPROM.put(0, phy_count_temp);
    return true;
  }
  return false;
}
void update_battery(bool force)
{
  if(force)
  batterylevel =((float)analogRead(A0)) * 4.53;
  else
    batterylevel = (float)batterylevel * .97 + .03 * ((float)analogRead(A0)) * 4.53;
}
void update_stats()
{
  update_battery(false);
  signal_rssi = 0;
  if (Cellular.ready())
  {
    CellularSignal sig = Cellular.RSSI();
    signal_rssi = sig.getStrength();
    if (signal_rssi > 99)
      signal_rssi = 99;

    if (signal_rssi < 0)
      signal_rssi = 0;
  }
}

void update_publish(void)
{
  static unsigned int phy_count_published = 0;
  static unsigned int web_count_published = 0;
  static unsigned int web_users_published = 0;
  static unsigned int phy_users_published = 0;
  static system_tick_t last_publish_time = 0;

  bool update_now = false;
  if (millis() - last_publish_time > (1000 * 30 * 30))
  {
    update_now = true;
  }

  if (phy_count_published != phy_count || web_count_published != web_count || phy_users_published != phy_users || web_users_published != web_users || update_now)
  {

    if (millis() - last_publish_time > 2 * PUBLISH_TIME)
    {
      if (millis() > PUBLISH_TIME)
        last_publish_time = millis() - PUBLISH_TIME / 2;
    }

    if ((update_now || millis() - last_publish_time > PUBLISH_TIME) && Particle.connected())
    {
      update_stats();

      char msg[50]; // 4*10 + 2 + 2 + 2

      sprintf(msg, "%u,%u,%u,%u,%u,%u", phy_count, web_count, phy_users, web_users, batterylevel, signal_rssi);
      Particle.process();
      Particle.publish("msg", msg, NO_ACK);
      phy_count_published = phy_count;
      web_count_published = web_count;
      web_users_published = web_users;
      phy_users_published = phy_users;
      last_publish_time = millis();
      blue_led_handler(1);
    }
  }
}

void handle_web_count_todo(void)
{
  // slowly move web_count_todo into webcount at web_rate_todo
  // toggle web_pressed on and off like someone is pressing a button
  if (millis() - web_count_time > web_rate_todo)
  {
    if (web_pressed == false)
    {
      if (web_count_todo > 0)
      {
        web_pressed = true;
        web_count_todo--;
        web_count++;
        web_count_time = millis();
      }
    }
    else if (web_pressed == true)
    {
      web_pressed = false;
      web_count_time = millis();
    }
  }
}

void decay_rank()
{
  static uint32_t rank_time = 0;
  if (millis() - rank_time > 500)
  {
    ranking *= .8;
    rank_time = millis();
  }
}

void update_info(void){
    update_stats();
    char line1[30];
    display.clearDisplay();
    display.setCursor(0, 0);
    sprintf(line1, "Bat %.2fV", ((float)batterylevel) / 1000);
    display.print(line1);
    sprintf(line1, "Signal %%%d", signal_rssi);
    display.setCursor(0, 16);
    display.print(line1);
    display.display();
    Particle.process();

}

void update_loop(void)
{
  update_battery(false);
  Particle.process();

  handle_web_count_todo();

  phy_active = (millis() - phy_count_time > PHY_USER_TIMEOUT) ? false : true;
  web_active = (millis() - web_count_time > WEB_USER_TIMEOUT) ? false : true;

  if (!web_active)
    web_users = 0;
  phy_users = phy_active ? 1 : 0;

  red_led_handler();

  // publish last
  update_publish();
  blue_led_handler(0);

  decay_rank();

  while (millis() - phy_count_time > 5000 && digitalRead(D2) == FALSE)
  {
    update_info();
  }
}

// Open a serial terminal and see the IP address printed out
void subscription_button(const char *topic, const char *data)
{
  // if we have extra presses to do, immediately do them
  if (web_count_todo > 0)
  {
    web_count += web_count_todo;
    web_count_todo = 0;
  }
  unsigned int web_count_immediate = 0;

  // save new data in todo list
  sscanf(data, "%u,%u,%u,%u", &web_count_todo, &web_rate_todo, &web_count_immediate, &web_users);
  web_count += web_count_immediate;

  blue_led_handler(2); // blink twice
}

int subscription_text(String command)
{
  if (strlen(command) < (sizeof(web_text) / sizeof(char)) - 1)
    strcpy(web_text, command);

  blue_led_handler(2); // blink twice
  return 1;
}

void setup(void)
{
  display.begin(0x2, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
  pinMode(D2, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  pinMode(D4, OUTPUT);
  attachInterrupt(D2, button_press, FALLING);
  Particle.subscribe("web_button", subscription_button);
  Particle.function("web_text", subscription_text);
  Particle.publishVitals(3600);
  sprintf(temp_string, "Users Online: 0");
  eeprom_load();
  display_reinit();
  update_battery(true);
  while( !Particle.connected() &&  digitalRead(D2) == TRUE){
        update_info();
  }

}

void do_housekeeping(void)
{

  // only do one task each housekeeping call to hide blocking from the user
  // listed in order of priority
  if (eeprom_store_phy())
    return; // 100ms
  if (eeprom_store_web())
    return;         // 100ms
  display_reinit(); // 4ms
}

int refresh_count_text(char *count_text)
{
  itoa(phy_count + web_count, count_text, 10);
  return ((128 - (strlen(count_text) * 12)) / 2);
}

void slide_animation()
{
  // hide a housekeeping task at the start of the slide animation
  do_housekeeping();

  // slide animation for count text
  for (int i = 0; i < 17; i++)
  {
    display.clearDisplay();

    display.setCursor(refresh_count_text(count_text), flip_display ? i : 16 - i);
    display.print(count_text);

    display.display();

    update_loop();
  }
  // now that slide is complete flip the display
  flip_display = !flip_display;

  // hide a housekeeping task at the end of the slide animation
  do_housekeeping();
}

void loop(void)
{
  // animation loop
  static bool display_web_text = true;
  static bool display_url = true;
  static bool display_users = false;
  static int display_message_index = 0;

  static unsigned int displayed_users = 0;

  // lookup current message
  const char *message;
  if (display_users)
  {
    displayed_users = web_users + phy_users;
    sprintf(temp_string, "Users Online: %u", displayed_users);
    message = temp_string;
  }
  else
  {
    if (display_url)
    {
      if (strlen(web_text) > 0 && display_web_text)
      {
        message = web_text;
        display_web_text = false;
      }
      else
      {
        message = string_display_url;
        display_web_text = true;
      }
    }
    else
    {
      message = string_table[display_message_index];
    }
  }

  char display_text[128] = {0};
  strcpy(display_text, message);

  // characters are 6 pixels wide
  // only animate 96 pixels past our destination, this is 32 pixels short
  // but the time will be taken up by the reinit done each cycle

  // scroll selected display message
  for (unsigned int i = 0; i < (strlen(display_text) * 6) + 96; i++)
  {
    display.clearDisplay();

    display.setCursor(refresh_count_text(count_text), flip_display ? 0 : 16);
    display.print(count_text);

    display.setCursor(128 - (i * 2), flip_display ? 16 : 0);
    display.print(display_text);

    display.display();

    update_loop();
  }

  slide_animation();

  if (millis() - phy_count_time < 20000)
  { // check if phrase has been said
    uint32_t start_time = millis();
    while (millis() - start_time < 20000)
    {
      display.clearDisplay();

      display.setCursor(refresh_count_text(count_text), flip_display ? 0 : 16);
      display.print(count_text);

      if (millis() - start_time < 2000)
      {
        sprintf(temp_string, "You Rate");
      }
      else
      {
        if (ranking < 10000)
          sprintf(temp_string, "Slow");
        else if (ranking < 20000 && ranking > 12000)
          sprintf(temp_string, "Normal");
        else if (ranking > 22000)
          sprintf(temp_string, "Turbo");
      }

      display.setCursor((128 - (strlen(temp_string) * 12)) / 2, flip_display ? 16 : 0);
      display.print(temp_string);

      display.display();

      update_loop();
    }
    slide_animation();
  }

  static int messages_displayed_counter = 0;

  // next message logic
  if (!display_url && !display_users)
  {
    display_message_index++;
    if (display_message_index > 16)
      display_message_index = 0;
    messages_displayed_counter++;
  }

  if (display_users)
  {
    display_users = false;
  }
  else
  {
    display_url = false;
  }

  if (phy_active)
  {
    if (messages_displayed_counter >= MESSAGE_LIMIT_ACTIVE)
    {
      display_url = true;
      messages_displayed_counter = 0;
    }
  }
  else
  {
    if (messages_displayed_counter >= MESSAGE_LIMIT_IDLE)
    {
      display_url = true;
      display_users = true;
      messages_displayed_counter = 0;
    }
  }
  if (displayed_users != (web_users + phy_users))
  {
    display_users = true;
  }
}
