/*
||
|| @file 	inflection.pde
|| @version	0.1
|| @author	Danne Stayskal
|| @contact	danne@stayskal.com
||
|| @description
|| | This is the production software that runs the visual effects for the 
|| | Inflection honorarium art project for Burning Man 2015
|| #
||
|| @changelog
|| | 0.1 2015-06-18 - Danne Stayskal : Initial Prototype
|| #
||
*/

#include "FastLED.h"

// Provision the LED interface
#define NUM_LEDS 180
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

// Provision the sensor interfaces
#define AUDIO_PIN 1
#define VIBRATION_PIN 5
int audio_level = 0;
int audio_buffer = 0;
int vibration_level = 0;

// Starting position and speed for the wave
int block_start = 0;
int block_speed = 1;

// Length and shape of the block 
int block_length = 10;
int block_shape[10] = { 1, 4, 10, 27, 90, 90, 27, 10, 4, 1 };

/* 
==========================================
= setup()
= Initializes needed variables and drivers, called by Arduino once during initialization.
*/
void setup() {
  // Initialize the LED interface
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}


/* 
==========================================
= loop()
= The main runtime heartbeat, called by Arduino once whenever the previous run of loop() completes.
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
      audio_level = audio_level - 5;
    }
  }
  
  // Clip it
  if (audio_level > 255) {
    audio_level = 255;
  }
  
  // Read and normalize the current vibration sensor input
  vibration_level = analogRead(VIBRATION_PIN);
  vibration_level = vibration_level / 10;
  
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
    leds[i].green = block_shape[(i+block_start) % block_length] * (vibration_level / 80.0);
  }
  
  // Push this frame to the LEDs
  FastLED.show();
  delay(10);

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
