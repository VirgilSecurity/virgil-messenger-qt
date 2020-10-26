//  Copyright (C) 2015-2020 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include "KeyboardEventFilter.h"

#include <QGuiApplication>
#include <QQuickItem>

using namespace VSQ;

KeyboardEventFilter::KeyboardEventFilter(QObject *parent)
    : QObject(parent)
    , m_inputMethod(qApp->inputMethod())
{
    connect(m_inputMethod, &QInputMethod::keyboardRectangleChanged, this, &KeyboardEventFilter::updateKeyboardRectangle);
}

KeyboardEventFilter::~KeyboardEventFilter()
{}

void KeyboardEventFilter::install(QQuickItem *item)
{
    item->installEventFilter(this);
}

bool KeyboardEventFilter::eventFilter(QObject *obj, QEvent *event)
{
#ifdef VS_IOS
    if (event->type() == QEvent::InputMethodQuery) {
        // HACK(fpohtmeh): Qt scrolls up entire root view if input item is overlapped by keyboard.
        // Set empty cursor rectangle to avoid scrolling.
        auto imEvent = static_cast<QInputMethodQueryEvent *>(event);
        if (imEvent->queries() == Qt::ImCursorRectangle) {
            imEvent->setValue(Qt::ImCursorRectangle, QRectF());
            return true;
        }
    }
#endif
    return QObject::eventFilter(obj, event);
}

void KeyboardEventFilter::updateKeyboardRectangle()
{
    const auto rect = m_inputMethod->keyboardRectangle();
    m_keyboardRectangle = rect;
    emit keyboardRectangleChanged(rect);
}
