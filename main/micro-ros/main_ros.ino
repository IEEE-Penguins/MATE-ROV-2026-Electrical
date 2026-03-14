#include <Arduino.h>
#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h> // Change this if using Int32, Float32, etc.

// --- 1. GLOBAL ENTITIES (The Skeleton) ---
rclc_support_t support;       // The Session Manager (Clock & Connection)
rcl_node_t node;              // The Identity (The Name on the Network)
rcl_allocator_t allocator;    // The Memory Manager (Handles RAM)
rclc_executor_t executor;     // The Worker (Listens for data in the loop)

// --- 2. COMMUNICATION ENTITIES ---
rcl_subscription_t subscriber; // The "Ear" (Input)
std_msgsmsgString msg;     // The "Buffer" (Storage for incoming data)

// --- 3. ERROR HANDLING ---
// If a micro-ROS function fails, the LED blinks fast and the code stops.
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define LED_BUILTIN 2

void error_loop(){
  while(1){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

// --- 4. THE CALLBACK (The Action) ---
void subscription_callback(const void * msgin) {
  const std_msgsmsgString * incoming_msg = (const std_msgsmsgString *)msgin;
  // logic goes here (e.g., if data == "1" then move servo)
}
void setup() {
  // A. TRANSPORT: Switch Serial to XRCE-DDS Binary Mode
  set_microros_transports();
  pinMode(LED_BUILTIN, OUTPUT);

  delay(2000); // Give the Agent time to start

  // B. ALLOCATOR: Setup RAM management
  allocator = rcl_get_default_allocator();

  // C. SUPPORT: Handshake with the Agent (PC)
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // D. NODE: Define the Node Name
  RCCHECK(rclc_node_init_default(&node, "base_reference_node", "", &support));

  // E. SUBSCRIBER: Define the Topic Name and Type
  RCCHECK(rclc_subscription_init_default(
    &subscriber, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "base/command"));

  // F. EXECUTOR: Initialize the worker and link the callback
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));

  // G. STATIC MEMORY: Reserve bytes for the incoming binary stream
  msg.data.capacity = 100; 
  msg.data.data = (char) malloc(msg.data.capacity sizeof(char));
}

void loop() {
  // H. THE HEARTBEAT: Keep the binary stream flowing
  // This MUST run constantly. Avoid long delay() calls!
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}