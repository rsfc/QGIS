/***************************************************************************
                              qgstextannotationdialog.cpp
                              ---------------------------
  begin                : February 24, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextannotationdialog.h"
#include "qgsannotationwidget.h"
#include "qgstextannotation.h"
#include "qgsmapcanvasannotationitem.h"
#include "qgsannotationmanager.h"
#include "qgsproject.h"
#include <QColorDialog>
#include <QGraphicsScene>

QgsTextAnnotationDialog::QgsTextAnnotationDialog( QgsMapCanvasAnnotationItem *item, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mItem( item )
  , mTextDocument( nullptr )
{
  setupUi( this );
  mEmbeddedWidget = new QgsAnnotationWidget( mItem );
  mStackedWidget->addWidget( mEmbeddedWidget );
  mStackedWidget->setCurrentWidget( mEmbeddedWidget );
  connect( mEmbeddedWidget, &QgsAnnotationWidget::backgroundColorChanged, this, &QgsTextAnnotationDialog::backgroundColorChanged );
  mTextEdit->setAttribute( Qt::WA_TranslucentBackground );
  if ( mItem && mItem->annotation() )
  {
    QgsTextAnnotation *annotation = static_cast< QgsTextAnnotation * >( mItem->annotation() );
    mTextDocument.reset( annotation->document() ? annotation->document()->clone() : nullptr );
    mTextEdit->setDocument( mTextDocument.get() );
  }

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setAllowOpacity( true );
  mFontColorButton->setContext( QStringLiteral( "symbology" ) );

  setCurrentFontPropertiesToGui();

  QObject::connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsTextAnnotationDialog::applyTextToItem );
  QObject::connect( mFontComboBox, &QFontComboBox::currentFontChanged, this, &QgsTextAnnotationDialog::changeCurrentFormat );
  QObject::connect( mFontSizeSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsTextAnnotationDialog::changeCurrentFormat );
  QObject::connect( mBoldPushButton, &QPushButton::toggled, this, &QgsTextAnnotationDialog::changeCurrentFormat );
  QObject::connect( mItalicsPushButton, &QPushButton::toggled, this, &QgsTextAnnotationDialog::changeCurrentFormat );
  QObject::connect( mTextEdit, &QTextEdit::cursorPositionChanged, this, &QgsTextAnnotationDialog::setCurrentFontPropertiesToGui );

  QPushButton *deleteButton = new QPushButton( tr( "Delete" ) );
  QObject::connect( deleteButton, &QPushButton::clicked, this, &QgsTextAnnotationDialog::deleteItem );
  mButtonBox->addButton( deleteButton, QDialogButtonBox::RejectRole );
}

void QgsTextAnnotationDialog::showEvent( QShowEvent * )
{
  backgroundColorChanged( mItem && mItem->annotation() && mItem->annotation()->fillSymbol() ? mItem->annotation()->fillSymbol()->color() : Qt::white );
}

void QgsTextAnnotationDialog::on_mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    applyTextToItem();
    mEmbeddedWidget->apply();
  }
}

void QgsTextAnnotationDialog::backgroundColorChanged( const QColor &color )
{
  QPalette p = mTextEdit->viewport()->palette();
  p.setColor( QPalette::Base, color );
  mTextEdit->viewport()->setPalette( p );
}

void QgsTextAnnotationDialog::applyTextToItem()
{
  if ( mItem && mTextDocument && mItem->annotation() )
  {
    QgsTextAnnotation *annotation = static_cast< QgsTextAnnotation * >( mItem->annotation() );
    //apply settings from embedded item widget
    if ( mEmbeddedWidget )
    {
      mEmbeddedWidget->apply();
    }
    annotation->setDocument( mTextDocument.get() );
    mItem->update();
  }
}

void QgsTextAnnotationDialog::changeCurrentFormat()
{
  QFont newFont;
  newFont.setFamily( mFontComboBox->currentFont().family() );

  //bold
  if ( mBoldPushButton->isChecked() )
  {
    newFont.setBold( true );
  }
  else
  {
    newFont.setBold( false );
  }

  //italic
  if ( mItalicsPushButton->isChecked() )
  {
    newFont.setItalic( true );
  }
  else
  {
    newFont.setItalic( false );
  }

  //size
  newFont.setPointSize( mFontSizeSpinBox->value() );
  mTextEdit->setCurrentFont( newFont );

  //color
  mTextEdit->setTextColor( mFontColorButton->color() );
}

void QgsTextAnnotationDialog::on_mFontColorButton_colorChanged( const QColor &color )
{
  Q_UNUSED( color )
  changeCurrentFormat();
}

void QgsTextAnnotationDialog::setCurrentFontPropertiesToGui()
{
  blockAllSignals( true );
  QFont currentFont = mTextEdit->currentFont();
  mFontComboBox->setCurrentFont( currentFont );
  mFontSizeSpinBox->setValue( currentFont.pointSize() );
  mBoldPushButton->setChecked( currentFont.bold() );
  mItalicsPushButton->setChecked( currentFont.italic() );
  mFontColorButton->setColor( mTextEdit->textColor() );
  blockAllSignals( false );
}

void QgsTextAnnotationDialog::blockAllSignals( bool block )
{
  mFontComboBox->blockSignals( block );
  mFontSizeSpinBox->blockSignals( block );
  mBoldPushButton->blockSignals( block );
  mItalicsPushButton->blockSignals( block );
  mFontColorButton->blockSignals( block );
}

void QgsTextAnnotationDialog::deleteItem()
{
  if ( mItem && mItem->annotation() )
    QgsProject::instance()->annotationManager()->removeAnnotation( mItem->annotation() );
  mItem = nullptr;
}

