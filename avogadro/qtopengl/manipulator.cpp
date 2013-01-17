/******************************************************************************

  This source file is part of the Avogadro project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "editor.h"

#include "glwidget.h"

#include <avogadro/core/vector.h>
#include <avogadro/qtgui/molecule.h>
#include <avogadro/rendering/camera.h>
#include <avogadro/rendering/glrenderer.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

using Avogadro::Core::Atom;
using Avogadro::Core::Bond;
using Avogadro::QtGui::Molecule;

using Avogadro::Rendering::Primitive;

namespace Avogadro {
namespace QtOpenGL {

Manipulator::Manipulator(GLWidget *widget)
  : m_glWidget(widget),
    m_molecule(0),
    m_object(Primitive::Identifier()),
    m_pressedButtons(Qt::NoButton)
{
}

Manipulator::~Manipulator()
{
}

void Manipulator::mousePressEvent(QMouseEvent *e)
{
  updatePressedButtons(e, false);
  m_lastMousePosition = e->pos();

  if (m_pressedButtons & Qt::LeftButton) {
    m_object = m_glWidget->renderer().hit(e->pos().x(), e->pos().y());

    if (m_object.molecule != m_molecule) {
      e->ignore();
      return;
    }

    switch (m_object.type) {
    case Primitive::Invalid:
      e->ignore();
      return;
    case Primitive::Atom:
      e->accept();
      return;
    case Primitive::Bond:
      Bond bond = m_molecule->bond(m_object.index);
      bond.setOrder((bond.order() % static_cast<unsigned char>(3))
                    + static_cast<unsigned char>(1));
      emit moleculeChanged();
      e->accept();
      return;
    }
  }
  else if (m_pressedButtons & Qt::RightButton) {
    // Delete the current primitive
    m_object = m_glWidget->renderer().hit(e->pos().x(), e->pos().y());

    if (m_object.molecule != m_molecule) {
      e->ignore();
      return;
    }

    switch (m_object.type) {
    case Primitive::Invalid:
      e->ignore();
      return;
    case Primitive::Atom:
      e->ignore();
      return;
    case Primitive::Bond:
      e->ignore();
      return;
    }
  }
}

void Manipulator::mouseReleaseEvent(QMouseEvent *e)
{
  updatePressedButtons(e, true);
  e->ignore();
  if (m_object.type != Primitive::Invalid) {
    resetObject();
    e->accept();
  }
}

void Manipulator::mouseMoveEvent(QMouseEvent *e)
{
  e->ignore();
  if (m_pressedButtons & Qt::LeftButton) {
    if (m_object.type == Primitive::Atom) {
      if (m_object.molecule == m_molecule) {
        // Update atom position
        Atom atom = m_molecule->atom(m_object.index);
        Vector2f windowPos(e->posF().x(), e->posF().y());
        Vector3f oldPos(atom.position3d().cast<float>());
        Vector3f newPos = m_glWidget->renderer().camera().unProject(windowPos,
                                                                    oldPos);
        atom.setPosition3d(newPos.cast<double>());
        emit moleculeChanged();
        e->accept();
      }
    }
  }
}

void Manipulator::mouseDoubleClickEvent(QMouseEvent *e)
{
  e->ignore();
}

void Manipulator::wheelEvent(QWheelEvent *e)
{
  e->ignore();
}

void Manipulator::keyPressEvent(QKeyEvent *e)
{
  e->ignore();
}

void Manipulator::keyReleaseEvent(QKeyEvent *e)
{
  e->ignore();
}

void Manipulator::updatePressedButtons(QMouseEvent *e, bool release)
{
  /// @todo Use modifier keys on mac
  if (release)
    m_pressedButtons &= e->buttons();
  else
    m_pressedButtons |= e->buttons();
}

} // namespace QtOpenGL
} // namespace Avogadro