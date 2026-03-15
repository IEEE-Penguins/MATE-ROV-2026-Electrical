#include <Arduino.h>
#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

#define DEBUG_LED 13
#define LED_BUILTIN 2

rcl_subscription_t subscriber;
std_msgs__msg__String msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if ((temp_rc != RCL_RET_OK)) { error_loop(); } }

void error_loop() {
  while (1) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

void subscription_callback(const void * msgin) {
  const std_msgs__msg__String * incoming_msg = (const std_msgs__msg__String *)msgin;

  if (incoming_msg->data.data == NULL) {
    return;
  }

  String data = String(incoming_msg->data.data);

  if (data.indexOf("1") != -1) {
    digitalWrite(DEBUG_LED, HIGH);
  } else if (data.indexOf("0") != -1) {
    digitalWrite(DEBUG_LED, LOW);
  }
}

void setup() {
  set_microros_transports();

  pinMode(DEBUG_LED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(DEBUG_LED, LOW);

  delay(2000);
  allocator = rcl_get_default_allocator();

  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
  RCCHECK(rclc_node_init_default(&node, "debug_node", "", &support));

  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "rov/command"
  ));

  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
  RCCHECK(rclc_executor_add_subscription(
    &executor,
    &subscriber,
    &msg,
    &subscription_callback,
    ON_NEW_DATA
  ));

  msg.data.data = (char *)malloc(100 * sizeof(char));
  msg.data.capacity = 100;
  msg.data.size = 0;
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}
