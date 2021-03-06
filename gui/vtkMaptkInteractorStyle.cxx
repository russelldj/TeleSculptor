/*ckwg +29
 * Copyright 2018 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name Kitware, Inc. nor the names of any contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// maptk includes
#include "vtkMaptkInteractorStyle.h"

#include "Utils.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkMaptkInteractorStyle);

//----------------------------------------------------------------------------
vtkMaptkInteractorStyle::vtkMaptkInteractorStyle()
{
  this->AddObserver(
    vtkCommand::TimerEvent, this, &vtkMaptkInteractorStyle::TimerCallback);
}

//----------------------------------------------------------------------------
void vtkMaptkInteractorStyle::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkMaptkInteractorStyle::TimerCallback(vtkObject* vtkNotUsed(caller),
                                            unsigned long vtkNotUsed(eventId),
                                            void* callData)
{
  int timerId = (callData ? *(reinterpret_cast<int*>(callData)) : 1);
  if (timerId != this->TimerId)
  {
    // This is a timer event for another timer that we are not interested in.
    return;
  }
  // If the time is up, destroy the timer and reset timer Id.
  this->DestroyTimer();
}

//----------------------------------------------------------------------------
void vtkMaptkInteractorStyle::DestroyTimer()
{
  this->Interactor->DestroyTimer(this->TimerId);
  this->TimerId = -1;
  this->TimerStatus = TimedOut;
}

//----------------------------------------------------------------------------
void vtkMaptkInteractorStyle::OnLeftButtonDown()
{
  int* eventPos = this->Interactor->GetEventPosition();

  if (this->TimerStatus == TimedOut)
  {
    // This is the first click. Create timer.
    this->TimerId =
      this->Interactor->CreateOneShotTimer(GetDoubleClickInterval());
    this->TimerStatus = Timing;
  }
  else
  {
    // Still timing, i.e. double clicked
    this->DestroyTimer();
    vtkRenderer* ren =
      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    if (ren)
    {
      double worldPt[3];
      vtkNew<vtkPointPicker> picker;
      picker->SetTolerance(0.01);
      int picked = picker->Pick(eventPos[0], eventPos[1], 0, ren);
      picker->GetPickPosition(worldPt);
      if (!picked)
      {
        ren->SetDisplayPoint(eventPos[0], eventPos[1], 0);
        ren->DisplayToWorld();
        double* wPt = ren->GetWorldPoint();
        worldPt[0] = wPt[0];
        worldPt[1] = wPt[1];
        worldPt[2] = wPt[2];
      }

      vtkCamera* cam = ren->GetActiveCamera();
      cam->SetFocalPoint(worldPt);

      cam->OrthogonalizeViewUp();
      ren->ResetCameraClippingRange();
    }
  }

  // Forward events
  this->Superclass::OnLeftButtonDown();
}
