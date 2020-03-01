/*
    Copyright 2014-2015 Harald Sitter <sitter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOURCE_H
#define SOURCE_H

#include "device.h"
#include "pulse/pulseaudio.h"

namespace QPulseAudio
{

class Source : public Device
{
    Q_OBJECT
    Q_PROPERTY(quint32 inputVolume READ inputVolume NOTIFY inputVolumeChanged)
public:
    explicit Source(QObject *parent);
    void update(const pa_source_info *info);
    void setVolume(qint64 volume) override;
    void setMuted(bool muted) override;
    void setActivePortIndex(quint32 port_index) override;
    void setChannelVolume(int channel, qint64 volume) override;
    pa_stream* stream();
    pa_stream* setStream(pa_stream*);
    quint32 inputVolume();
    void setSignalPowerLevel(float vol);
    bool isDefault() const override;
    void setDefault(bool enable) override;
    void switchStreams() override;
    static void readCallback(pa_stream *s, size_t length, void *userdata);

Q_SIGNALS:
    void inputVolumeChanged();

private:
    pa_stream *m_stream;
    float m_inputVolume;
};

} // QPulseAudio

#endif // SOURCE_H
