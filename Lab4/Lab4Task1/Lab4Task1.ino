// Total times for tasks
const TickType_t ledTaskExecutionTime = 500 / portTICK_PERIOD_MS;      // 500 ms
const TickType_t counterTaskExecutionTime = 2000 / portTICK_PERIOD_MS; // 2 seconds
const TickType_t alphabetTaskExecutionTime = 13000 / portTICK_PERIOD_MS; // 13 seconds
// Remaining Execution Times
volatile TickType_t remainingLedTime = ledTaskExecutionTime;
volatile TickType_t remainingCounterTime = counterTaskExecutionTime;
volatile TickType_t remainingAlphabetTime = alphabetTaskExecutionTime;


void ledTask(void *arg) {
   // TODO: Blink an LED and update remaining time for this task
}


void counterTask(void *arg) {
 // TODO: Print out an incrementing counter to your LCD, and 
 //       update remaining time for this task
}


void alphabetTask(void *arg) {
 // TODO: Print out the alphabet to Serial, and update remaining
 //       time for this task
}


void scheduleTasks(void *arg) {
   // TODO: Implement SRTF scheduling logic. This function should select the task with 
   //       the shortest remaining time and run it. Once a task completes it should 
   //       reset its remaining time.
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
