/*
||
|| @file 	inflection.pde
|| @version	0.2
|| @author	Danne Stayskal
|| @contact	danne@stayskal.com
||
|| @description
|| | This is the production software that runs the visual effects for the 
|| | Inflection honorarium art project for Burning Man 2015
|| #
||
|| @changelog
|| | 0.1   2015-06-18 - Danne Stayskal : Initial Prototype
|| | 0.2   2015-06-25 - Danne Stayskal : Adding accelerometer and relay support, improved animations
|| | 0.2.1 2015-06-25 - Danne Stayskal : Adding sync code for floods on relays
|| #
||
*/

#include "FastLED.h"
#include "Metro.h"

// Provision the LED interface
#define NUM_LEDS 180
#define DATA_PIN 13
#define CLOCK_PIN 12
CRGB leds[NUM_LEDS];

// Provision the audio interface
#define AUDIO_PIN 1
int audio_level = 0;
int audio_buffer = 0;

// Provision the accelerometer interfaces
#define ACCEL_X_PIN 2
#define ACCEL_Y_PIN 3
#define ACCEL_Z_PIN 4
int accel_x = 0;        // Acceleration due to gravity
int accel_y = 0;
int accel_z = 0;
int accel_x_buffer = 0; 
int accel_y_buffer = 0;
int accel_z_buffer = 0;
int accel_x_delta = 0;  // Acceleration due to hippies
int accel_y_delta = 0;
int accel_z_delta = 0;
int accel_x_prev = 0;
int accel_y_prev = 0;
int accel_z_prev = 0;
int accel_scalar = 0;   // Scalar composite 2nd differential

// Provision the relay interfaces
#define RELAY_CH1_PIN 2
#define RELAY_CH2_PIN 3
#define RELAY_CH3_PIN 4
#define RELAY_CH4_PIN 5

int relay_mute_phase = 0;

// Starting position and speed for the wave
int block_start = 0;
int block_speed = 1;

// Length and shape of the block 
int block_length = 10;
int block_shape[10] = { 1, 4, 10, 27, 90, 90, 27, 10, 4, 1 };

/* 
|| setup()
|| Initializes needed variables and drivers, called by Arduino once during initialization.
*/
void setup() {

  // Initialize the LED interface
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
  
  // Initialize our relay interfaces
  pinMode(RELAY_CH1_PIN, OUTPUT);
  pinMode(RELAY_CH2_PIN, OUTPUT);
  pinMode(RELAY_CH3_PIN, OUTPUT);
  pinMode(RELAY_CH4_PIN, OUTPUT);

  // Turn off each of the four floods
  digitalWrite(RELAY_CH1_PIN, LOW);
  digitalWrite(RELAY_CH2_PIN, LOW);
  digitalWrite(RELAY_CH3_PIN, LOW);
  digitalWrite(RELAY_CH4_PIN, LOW);

}


/* 
|| loop()
|| The main runtime heartbeat, called by Arduino once whenever the previous run of loop() completes.
*/
void loop() {

  // Read, normalize, and average the current audio sensor levels
  audio_buffer = analogRead(AUDIO_PIN);
  audio_buffer = audio_buffer / 10;
  if (audio_buffer > 0){
    if (audio_buffer > audio_level) {
      // Louder than the last cycle. Peak.
      audio_level = audio_buffer;
    } else {
      // Softer than the last cycle. Decrement.
      audio_level = audio_level - 10;
    }
  }
  
  // Clip it
  if (audio_level > 255) {
    audio_level = 255;
  }
  
  // Read and normalize the current acceleration sensor input
  // These are all calibrated to 1..100
  accel_x_buffer = (analogRead(ACCEL_X_PIN) / 2) - 128;
  accel_y_buffer = (analogRead(ACCEL_Y_PIN) / 2) - 128;
  accel_z_buffer = (analogRead(ACCEL_Z_PIN) / 2) - 128;
  accel_x = ((1.0 * accel_x) + accel_x_buffer) / 2;
  accel_y = ((1.0 * accel_y) + accel_y_buffer) / 2;
  accel_z = ((1.0 * accel_z) + accel_z_buffer) / 2;

  // These are the second differential (rate of change of rate of change)
  // AKA acceleration due to hippies, not acceperation due to gravity.
  // Faking the quantized funk a touch here, since differential calculus isn't 
  // a strength of the Arduino platform.
  accel_x_delta = abs(accel_x - accel_x_prev);
  accel_y_delta = abs(accel_y - accel_y_prev);
  accel_z_delta = abs(accel_z - accel_z_prev);
  accel_x_prev = accel_x;
  accel_y_prev = accel_y;
  accel_z_prev = accel_z;
  
  // And finally, the composite scalar form of the second differential of 
  // measured gravitational acceleration. 1.44224 is roughly the cube root of three
  accel_scalar = ((accel_x_delta + accel_y_delta + accel_z_delta) / 1.44224)*2;
  
  // Clear the LED array
  for (int i = 0; i < 100; i++){
    leds[i] = CRGB::Black;
  }
  
  // Draw the sound sensor as twilight static on top
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB(random(audio_level/2), 0, random(audio_level));
  }
  
  // Draw the vibration sensor as green waves
  block_start = block_start + block_speed;
  if (block_start > block_length){
    block_start = 0;
  }
  for(int i = 0; i<NUM_LEDS; i++){
    leds[i].green = block_shape[(i+block_start) % block_length] * (accel_scalar / 80.0);
  }
  
  // Push this frame to the LEDs
  FastLED.show();
  delay(10);

  // Stagger the relays on one at a time every 3 seconds
  staggerRelayOn();
  
}


/* 
|| staggerRelayOn()
|| Turns on the next successive relay channel every 3 seconds.
*/
void staggerRelayOn() {
  static Metro onInterval(3000UL); // turn the next relay on every 3s.
  const byte relayPins[]={RELAY_CH1_PIN, RELAY_CH2_PIN, RELAY_CH3_PIN, RELAY_CH4_PIN };
  const byte nPins = sizeof(relayPins);
  static byte pin = 0; // start

  if( pin >= nPins ) return; // done with turning on.

  if( onInterval.check() ) { // time to turn the next pin on.
    pinMode(relayPins[pin], OUTPUT); // current source
    digitalWrite(relayPins[pin], HIGH); // on
    pin++;
  }
}


/*
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
==========================================
*/
