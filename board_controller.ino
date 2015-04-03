#include <SPI.h>

const int LATCH_PIN = 12; // Latch pin is pin 12 on the 74HC595 chip
const int OUTPUT_ENABLE_PIN = 10; // Output enable is pin 13 on the 74HC595 chip
// Master reset always high (VCC) - Pin 10 on the 74HC595 chip
// Data pin is 11 (SPI MOSI) on Arduino Uno - Pin 14 on the 74HC595 chip
// Clock pin is 13 (SPI CLK) on Arduino Uno - Pin 11 on the 74HC595 chip

const int ROW_LENGTH = 8;
const int NUM_ROWS = 8;
byte frame_buffer[ROW_LENGTH*NUM_ROWS];

enum spi_data { SPI_ROW = 0, SPI_BLUE, SPI_GREEN, SPI_RED };
enum channels { RED_CHANNEL = 1, GREEN_CHANNEL = 2, BLUE_CHANNEL = 4 };

byte color = 0;
unsigned long lastCycle_ms = 0;
unsigned long cycleDelay_ms = 500;
void setup() {
  pinMode( LATCH_PIN, OUTPUT );
  pinMode( OUTPUT_ENABLE_PIN, OUTPUT );
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV128); // For debugging, try DIV2 or DIV4 once code confirmed
  digitalWrite( OUTPUT_ENABLE_PIN, LOW );
}

// Redraw the board constantly, cycle through the colours periodically
void loop() {
  if( millis() >= lastCycle_ms + cycleDelay_ms )
  {
    lastCycle_ms += cycleDelay_ms;
    memset( frame_buffer, sizeof(frame_buffer), color );   
    color += 1;
    if( color > 7 ) color = 0;
  }
  renderBoard( frame_buffer, ROW_LENGTH, NUM_ROWS );
}

// 256 Cycles to update board
// At div 128 will take ~2ms to update board.
// At div 2, ~0.03ms to update board.
void renderBoard( byte frame_buffer[], int rowLength, int numRows )
{
  byte spi_buffer[4];
  for( int y = 0; y < numRows; ++y )
  {
    digitalWrite( LATCH_PIN, LOW );
    spi_buffer[SPI_ROW] = ~(1<<y); //Active row is low, inactive rows are high
    spi_buffer[SPI_BLUE] = 0;
    spi_buffer[SPI_GREEN] = 0;
    spi_buffer[SPI_RED] = 0;
    
    int row_start = y * rowLength;
    for( int x = 0; x < rowLength; ++x )
    {
      byte val = frame_buffer[row_start + x];
      if ( val & RED_CHANNEL != 0 ) spi_buffer[SPI_RED] |= 1 << x; 
      if ( val & GREEN_CHANNEL != 0 ) spi_buffer[SPI_GREEN] |= 1 << x;
      if ( val & BLUE_CHANNEL != 0 ) spi_buffer[SPI_BLUE] |= 1 << x;
    }
    for( int i = 0; i < sizeof(spi_buffer); ++i )
    {
      SPI.transfer(spi_buffer[i]);
    }
    digitalWrite( LATCH_PIN, HIGH );
  }
}
