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

#include "source.h"

#include "context.h"
#include "server.h"
#include "sourceoutput.h"
#include "math.h"

namespace QPulseAudio
{

Source::Source(QObject *parent)
    : Device(parent)
{
    m_stream = 0;
    connect(context()->server(), &Server::defaultSourceChanged, this, &Source::defaultChanged);
}

void Source::update(const pa_source_info *info)
{
    updateDevice(info);
}

void Source::setVolume(qint64 volume)
{
    context()->setGenericVolume(index(), -1, volume, cvolume(), &pa_context_set_source_volume_by_index);
}

void Source::setMuted(bool muted)
{
    context()->setGenericMute(index(), muted, &pa_context_set_source_mute_by_index);
}

void Source::setActivePortIndex(quint32 port_index)
{
    Port *port = qobject_cast<Port *>(ports().at(port_index));
    if (!port) {
        qCWarning(PLASMAPA) << "invalid port set request" << port_index;
        return;
    }
    context()->setGenericPort(index(), port->name(), &pa_context_set_source_port_by_index);
}

void Source::setChannelVolume(int channel, qint64 volume)
{
    context()->setGenericVolume(index(), channel, volume, cvolume(), &pa_context_set_source_volume_by_index);
}

bool Source::isDefault() const
{
    return context()->server()->defaultSource() == this;
}

void Source::setDefault(bool enable)
{
    if (!isDefault() && enable) {
        context()->server()->setDefaultSource(this);
    }
}

void Source::setSignalPowerLevel(float vol) {
    if (vol != m_inputVolume) {
        m_inputVolume = (quint32) std::round(vol);
        qWarning() << "Volume: " << vol;
        Q_EMIT inputVolumeChanged();
    }
}

void Source::readCallback(pa_stream *s, size_t length, void *userdata) {
    Source * source = static_cast<Source*>(userdata);
    const void * data;
    float vol;
    if (pa_stream_peek(s, &data, &length) < 0) {
        qWarning() << "Unable to read stream data";
        return;
    }
    //if (!data && length) {
    //    pa_stream_drop(s);
    //    return;
    //}
    
    float sum = 0;
    int samples = 50;
    if (length / sizeof(float) < 50) samples = length / sizeof(float);

    for (int i = 0; i < samples; i++)
        sum += ((const float *)data)[length / sizeof(float) - samples];
    
    vol = sum / samples;
    pa_stream_drop(s);
    if (vol < 0) vol = 0;
    if (vol > 1) vol = 1;

    vol *= 100;

    source->setSignalPowerLevel(vol);
}

quint32 Source::inputVolume() {
    return m_inputVolume;
}

pa_stream* Source::stream() {
    return m_stream;
}

pa_stream* Source::setStream(pa_stream* stream) {
    m_stream = stream;
    return m_stream;
}

void Source::switchStreams()
{
    auto data = context()->sourceOutputs().data();
    std::for_each(data.begin(), data.end(), [this](SourceOutput* paObj) {
        paObj->setDeviceIndex(m_index);
    });
}

} // QPulseAudio
