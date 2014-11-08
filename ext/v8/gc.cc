#include "rr.h"
#include <iostream>

namespace rr {
  GC::Queue* queue;
  VALUE root;
  std::map<VALUE,bool> retainedList;

  GC::Queue::Queue() : first(0), divider(0), last(0){
    first = new GC::Queue::Node(NULL);
    divider = first;
    last = first;
  }

  void GC::Queue::Enqueue(void* reference) {
    last->next = new Node(reference);
    last = last->next;
    while (first != divider) {
      Node* tmp = first;
      first = first->next;
      delete tmp;
    }
  }

  void* GC::Queue::Dequeue() {
    void* result = NULL;
    if (divider != last) {
      result = divider->next->value;
      divider = divider->next;
    }
    return result;
  }

  void GC::Finalize(void* phantom) {
    queue->Enqueue(phantom);
  }
  void GC::Drain(v8::GCType type, v8::GCCallbackFlags flags) {
    for(Phantom phantom = queue->Dequeue(); phantom.NotNull(); phantom = queue->Dequeue()) {
      phantom.destroy();
    }

  }
  void GC::Init() {
    queue = new GC::Queue();
    retainedList = std::map<VALUE, bool>();

    v8::V8::AddGCPrologueCallback(GC::Drain);

    root = Data_Wrap_Struct(rb_cObject, &GC::MarkRetainedRubyObjects, NULL, &retainedList);
    rb_gc_register_address(&root);
  }

  void GC::MarkRetainedRubyObjects(std::map<VALUE,bool>* list) {
    for(std::map<VALUE, bool>::iterator i = list->begin(); i != list->end(); i++) {
      VALUE object = i->first;
      bool isRetained = i->first;
      if (isRetained) {
        rb_gc_mark(object);
      } else {
        // remove?
      }
    }
  }

  void GC::RetainRubyObject(VALUE object) {
    retainedList[object] = true;
  }

  void GC::ReleaseRubyObject(VALUE object) {
    retainedList[object] = false;
  }
}
