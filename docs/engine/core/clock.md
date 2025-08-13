# Clock

## Definitions

The clock module is responsible for updating the time in the game engine and verifying the correctness of status updates over time.

## Structures

Clock is responsible for storing the start of the countdown an the amount of time that has elapsed for various intervals.

### Field

- f64 startTime
- f64 elapsed

## Functions

- void clockUpdate(Clock* clock) - Updated the provided clock.Should be called just before checking elapsed time. Has no effect on non-started clocks.

- void clockStart(Clock* clock) - Starts the provided clock. Resets elapsed time.

- void clockStop(Clock* clock) - Stops the provided clock. Does not reset elapsed time.
