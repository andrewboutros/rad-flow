// $Id$

/*
 Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*credit.cpp
 *
 *A class for credits
 */

#include "booksim.hpp"
#include "credit.hpp"

// Forward Declaration of the all and free static stacks. The free stack contains Credits that are no longer in use
// but can be reset and reused in the future. This avoids unnecessarily creating and destroying Credit pointers.
stack<Credit *> Credit::_all;
stack<Credit *> Credit::_free;

// Constructor simply calls the Reset() function.
Credit::Credit()
{
  Reset();
}

// Resets all Credit contents for future reuse or creating a new one from scratch.
void Credit::Reset()
{
  vc.clear();
  head = false;
  tail = false;
  id   = -1;
}

// Gets a pointer to an existing "free" Credit object to reuse. If none exist, create a new one from scratch
Credit * Credit::New() {
  Credit * c;
  if(_free.empty()) {
    c = new Credit();
    _all.push(c);
  } else {
    c = _free.top();
    c->Reset();
    _free.pop();
  }
  return c;
}

// Pushes a Credit pointer to the "free" stack such that it can be reused in the future.
void Credit::Free() {
  _free.push(this);
}

// Deletes all Credit pointers to free memory. This is done only once at the end of the program.
void Credit::FreeAll() {
  while(!_all.empty()) {
    delete _all.top();
    _all.pop();
  }
}

// Returns the number of Credits currently in circulation (i.e. Credits that are not "free").
int Credit::OutStanding(){
  return _all.size()-_free.size();
}
