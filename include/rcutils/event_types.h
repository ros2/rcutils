// Copyright 2020 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCUTILS__EVENT_TYPES_H_
#define RCUTILS__EVENT_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

enum EventQueueType {
    SUBSCRIPTION_EVENT,
    SERVICE_EVENT,
    CLIENT_EVENT,
    GUARD_CONDITION_EVENT
};

typedef enum EventQueueType EventQueueType;

struct EventQ {
  const void * entity;
  EventQueueType type;
};

typedef struct EventQ EventQ;

typedef void (*Event_callback)(const void * context, EventQ event);

struct EventHandle {

    // Associated context (executor)
    const void * context;

    // Event handle;
    const void * ros2_handle;

    // Event callback
    Event_callback callback;
};

typedef struct EventHandle EventHandle;


#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__EVENT_TYPES_H_