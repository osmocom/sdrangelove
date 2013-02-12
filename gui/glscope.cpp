#include <QPainter>
#include <QMouseEvent>
#include "glscope.h"
#include "../dsp/dspengine.h"

GLScope::GLScope(QWidget* parent) :
	QGLWidget(parent),
	m_changed(false),
	m_changesPending(true),
	m_mode(ModeIQ),
	m_oldTraceSize(-1),
	m_sampleRate(0),
	m_dspEngine(NULL),
	m_scopeVis(NULL),
	m_amp(1.0),
	m_timeStep(1),
	m_timeOfsProMill(0),
	m_triggerChannel(ScopeVis::TriggerFreeRun)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
	m_timer.start(50);
}

GLScope::~GLScope()
{
	if(m_dspEngine != NULL) {
		m_dspEngine->removeSink(m_scopeVis);
		delete m_scopeVis;
	}
}

void GLScope::setDSPEngine(DSPEngine* dspEngine)
{
	if((m_dspEngine == NULL) && (dspEngine != NULL)) {
		m_dspEngine = dspEngine;
		m_scopeVis = new ScopeVis(this);
		m_dspEngine->addSink(m_scopeVis);
	}
}

void GLScope::setAmp(Real amp)
{
	m_amp = amp;
	m_changed = true;
	update();
}

void GLScope::setTimeStep(int timeStep)
{
	m_timeStep = timeStep;
	m_changed = true;
	update();
}

void GLScope::setTimeOfsProMill(int timeOfsProMill)
{
	m_timeOfsProMill = timeOfsProMill;
	m_changed = true;
	update();
}

void GLScope::newTrace(const std::vector<Complex>& trace, int sampleRate)
{
	if(!m_mutex.tryLock(2))
		return;
	if(m_changed) {
		m_mutex.unlock();
		return;
	}

	switch(m_mode) {
		case ModeIQ:
			m_trace = trace;
			break;

		case ModeMagPha: {
			m_trace.resize(trace.size());
			std::vector<Complex>::iterator dst = m_trace.begin();
			for(std::vector<Complex>::const_iterator src = trace.begin(); src != trace.end(); ++src)
				*dst++ = Complex(abs(*src), arg(*src) / M_PI);
			break;
		}

		case ModeDerived12: {
			m_trace.resize(trace.size() - 3);
			std::vector<Complex>::iterator dst = m_trace.begin();
			for(int i = 3; i < trace.size() ; i++) {
				*dst++ = Complex(
					abs(trace[i] - trace[i - 1]),
					abs(trace[i] - trace[i - 1]) - abs(trace[i - 2] - trace[i - 3]));
			}
			break;
		}
	}

	m_sampleRate = sampleRate;
	m_changed = true;

	m_mutex.unlock();
}

void GLScope::initializeGL()
{
	glDisable(GL_DEPTH_TEST);
}

void GLScope::resizeGL(int width, int height)
{
	glViewport(0, 0, width, height);

	m_changesPending = true;
}

void GLScope::paintGL()
{
	if(!m_mutex.tryLock(2))
		return;

	if(m_changesPending)
		applyChanges();

	if(m_trace.size() != m_oldTraceSize) {
		m_oldTraceSize = m_trace.size();
		emit traceSizeChanged(m_trace.size());
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glScalef(2.0, -2.0, 1.0);
	glTranslatef(-0.50, -0.5, 0);

	// I

	// draw rect around
	glPushMatrix();
	glTranslatef(m_glScopeRectI.x(), m_glScopeRectI.y(), 0);
	glScalef(m_glScopeRectI.width(), m_glScopeRectI.height(), 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2f(1, 1);
	glVertex2f(0, 1);
	glVertex2f(0, 0);
	glVertex2f(1, 0);
	glEnd();
	glDisable(GL_BLEND);
	// paint grid
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 0.05);
	for(int i = 1; i < 10; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, i * 0.1);
		glVertex2f(1, i * 0.1);
		glEnd();
	}
	for(int i = 1; i < 10; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(i * 0.1, 0);
		glVertex2f(i * 0.1, 1);
		glEnd();
	}
	glPopMatrix();

	if(m_triggerChannel == ScopeVis::TriggerChannelI) {
		glPushMatrix();
		glTranslatef(m_glScopeRectI.x(), m_glScopeRectI.y() + m_glScopeRectI.height() / 2.0, 0);
		glScalef(m_glScopeRectI.width(), -(m_glScopeRectI.height() / 2) * m_amp, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glColor4f(0, 1, 0, 0.3f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, m_triggerLevelHigh);
		glVertex2f(1, m_triggerLevelHigh);
		glEnd();
		glColor4f(0, 0.8, 0.0, 0.3f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, m_triggerLevelLow);
		glVertex2f(1, m_triggerLevelLow);
		glEnd();
		glDisable(GL_LINE_SMOOTH);
		glPopMatrix();
	}

	if(m_trace.size() > 0) {
		glPushMatrix();
		glTranslatef(m_glScopeRectI.x(), m_glScopeRectI.y() + m_glScopeRectI.height() / 2.0, 0);
		glScalef(m_glScopeRectI.width() * (float)m_timeStep / (float)(m_trace.size() - 1), -(m_glScopeRectI.height() / 2) * m_amp, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glColor4f(1, 1, 0, 0.4f);
		int start = m_timeOfsProMill * (m_trace.size() - (m_trace.size() / m_timeStep)) / 1000;
		int end = start + m_trace.size() / m_timeStep;
		if(end - start < 2)
			start--;
		float prev = m_trace[start].real();
		float posLimit = 1.0 / m_amp;
		float negLimit = -1.0 / m_amp;
		if(prev > posLimit)
			prev = posLimit;
		else if(prev < negLimit)
			prev = negLimit;
		for(int i = start + 1; i < end; i++) {
			float v = m_trace[i].real();
			if(v > posLimit)
				v = posLimit;
			else if(v < negLimit)
				v = negLimit;
			glBegin(GL_LINE_LOOP);
			glVertex2f(i - 1 - start, prev);
			glVertex2f(i - start, v);
			glEnd();
			prev = v;
		}
		glDisable(GL_LINE_SMOOTH);
		glPopMatrix();
	}

	// Q

	// draw rect around
	glPushMatrix();
	glTranslatef(m_glScopeRectQ.x(), m_glScopeRectQ.y(), 0);
	glScalef(m_glScopeRectQ.width(), m_glScopeRectQ.height(), 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 0.5);
	glBegin(GL_LINE_LOOP);
	glVertex2f(1, 1);
	glVertex2f(0, 1);
	glVertex2f(0, 0);
	glVertex2f(1, 0);
	glEnd();
	glDisable(GL_BLEND);
	// paint grid
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1.0f);
	glColor4f(1, 1, 1, 0.05);
	for(int i = 1; i < 10; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, i * 0.1);
		glVertex2f(1, i * 0.1);
		glEnd();
	}
	for(int i = 1; i < 10; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex2f(i * 0.1, 0);
		glVertex2f(i * 0.1, 1);
		glEnd();
	}
	glPopMatrix();

	if(m_triggerChannel == ScopeVis::TriggerChannelQ) {
		glPushMatrix();
		glTranslatef(m_glScopeRectQ.x(), m_glScopeRectQ.y() + m_glScopeRectQ.height() / 2.0, 0);
		glScalef(m_glScopeRectQ.width(), -(m_glScopeRectQ.height() / 2) * m_amp, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glColor4f(0, 1, 0, 0.3f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, m_triggerLevelHigh);
		glVertex2f(1, m_triggerLevelHigh);
		glEnd();
		glColor4f(0, 0.8, 0.0, 0.3f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, m_triggerLevelLow);
		glVertex2f(1, m_triggerLevelLow);
		glEnd();
		glDisable(GL_LINE_SMOOTH);
		glPopMatrix();
	}

	if(m_trace.size() > 0) {
		glPushMatrix();
		glTranslatef(m_glScopeRectQ.x(), m_glScopeRectQ.y() + m_glScopeRectQ.height() / 2.0, 0);
		glScalef(m_glScopeRectQ.width() * (float)m_timeStep / (float)(m_trace.size() - 1), -(m_glScopeRectQ.height() / 2) * m_amp, 1);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);
		glColor4f(1, 1, 0, 0.4f);
		int start = m_timeOfsProMill * (m_trace.size() - (m_trace.size() / m_timeStep)) / 1000;
		int end = start + m_trace.size() / m_timeStep;
		if(end - start < 2)
			start--;
		float prev = m_trace[start].imag();
		float posLimit = 1.0 / m_amp;
		float negLimit = -1.0 / m_amp;
		if(prev > posLimit)
			prev = posLimit;
		else if(prev < negLimit)
			prev = negLimit;
		for(int i = start + 1; i < end; i++) {
			float v = m_trace[i].imag();
			if(v > posLimit)
				v = posLimit;
			else if(v < negLimit)
				v = negLimit;
			glBegin(GL_LINE_LOOP);
			glVertex2f(i - 1 - start, prev);
			glVertex2f(i - start, v);
			glEnd();
			prev = v;
		}
		glDisable(GL_LINE_SMOOTH);
		glPopMatrix();
	}

	glPopMatrix();
	m_changed = false;
	m_mutex.unlock();
}

void GLScope::mousePressEvent(QMouseEvent* event)
{
	int x = event->x() - 10;
	int y = event->y() - 10;
	Real time;
	Real amplitude;
	ScopeVis::TriggerChannel channel;
	if((x < 0) || (x >= width() - 20))
		return;
	if((y < 0) || (y >= height() - 20))
		return;

	if((m_sampleRate != 0) && (m_timeStep != 0) && (width() > 20))
		time = ((Real)x * (Real)m_trace.size()) / ((Real)m_sampleRate * (Real)m_timeStep * (Real)(width() - 20));
	else time = -1.0;

	if(y < (height() - 30) / 2) {
		channel = ScopeVis::TriggerChannelI;
		if((m_amp != 0) && (height() > 30))
			amplitude = 2.0 * ((height() - 30) * 0.25 - (Real)y) / (m_amp * (height() - 30) / 2.0);
		else amplitude = -1;
	} else if(y > (height() - 30) / 2 + 10) {
		y -= 10 + (height() - 30) / 2;
		channel = ScopeVis::TriggerChannelQ;
		if((m_amp != 0) && (height() > 30))
			amplitude = 2.0 * ((height() - 30) * 0.25 - (Real)y) / (m_amp * (height() - 30) / 2.0);
		else amplitude = -1;
	} else {
		channel = ScopeVis::TriggerFreeRun;
	}

	if(m_dspEngine != NULL) {
		qDebug("amp %f", amplitude);
		m_triggerLevelHigh = amplitude + 0.01 / m_amp;
		m_triggerLevelLow = amplitude - 0.01 / m_amp;
		if(m_triggerLevelHigh > 1.0)
			m_triggerLevelHigh = 1.0;
		else if(m_triggerLevelHigh < -1.0)
			m_triggerLevelHigh = -1.0;
		if(m_triggerLevelLow > 1.0)
			m_triggerLevelLow = 1.0;
		else if(m_triggerLevelLow < -1.0)
			m_triggerLevelLow = -1.0;
		m_scopeVis->configure(m_dspEngine->getMessageQueue(), channel, m_triggerLevelHigh, m_triggerLevelLow);
		m_triggerChannel = channel;
		m_changed = true;
		update();
	}
}

void GLScope::applyChanges()
{
	m_changesPending = false;

	m_glScopeRectI = QRectF(
		(float)10 / (float)width(),
		(float)10 / (float)height(),
		(float)(width() - 10 - 10) / (float)width(),
		(float)((height() - 10 - 10 - 10) / 2) / (float)height()
	);

	m_glScopeRectQ = QRectF(
		(float)10 / (float)width(),
		(float)(10 + 10 + (height() - 10 - 10 - 10) / 2) / (float)height(),
		(float)(width() - 10 - 10) / (float)width(),
		(float)((height() - 10 - 10 - 10) / 2) / (float)height()
	);
}

void GLScope::tick()
{
	if(m_changed)
		update();
}
