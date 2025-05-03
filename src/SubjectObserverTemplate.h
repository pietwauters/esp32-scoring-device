//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef SUBJECTOBSERVERTEMPLATE_H
#define SUBJECTOBSERVERTEMPLATE_H

#include <iostream>
#include <vector>
#include <string>
#include "EventDefinitions.h"

template <class T>
class Observer
   {
   public:
      Observer() {}
      virtual ~Observer() {}
      virtual void update(T *subject, uint32_t eventtype)= 0;
      virtual void update(T *subject, std::string eventtype){return;};
   };

template <class T>
class Subject
   {
   public:
      Subject() {}
      virtual ~Subject() {}
      void attach (Observer<T> &observer)
         {
         m_observers.push_back(&observer);
         }
      void notify (uint32_t eventtype = EVENT_ALL)
         {
         typename std::vector<Observer<T> *>::iterator it;
         for (it=m_observers.begin();it!=m_observers.end();it++) (*it)->update(static_cast<T *>(this), eventtype);
         }
      void notify (std::string eventtype = "")
            {
            typename std::vector<Observer<T> *>::iterator it;
            for (it=m_observers.begin();it!=m_observers.end();it++) (*it)->update(static_cast<T *>(this), eventtype);
            }
   private:
      std::vector<Observer<T> *> m_observers;
   };


   #endif // SUBJECTOBSERVERTEMPLATE_H
