/*
 Copyright 2010 Benjamin Loulier <http://www.benjaminloulier.com> <benlodotcom@gmail.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>

@class EAGLView;
@class Object3DParametersViewController;

@protocol ParametersDelegate

- (void)object3DParametersViewControllerWillDisplayModalViewController:(Object3DParametersViewController *)controller;
- (void)object3DParametersViewControllerWillDismissModalViewController:(Object3DParametersViewController *)controller;

@end

/*!
 @author Benjamin Loulier
 
 @brief	This class is the view controller for the parametersView
 
 The parametersView is a GUI where you can modify different display settings for your 3D Objects
 and see the results of these modifications on the way your object is displayed. You can the save 
 those settings and/or send them by mail to integrate them in another app for instance.
 */

@interface Object3DParametersViewController : UIViewController <MFMailComposeViewControllerDelegate> {
	@private
	id<ParametersDelegate> _delegate;
	//object
	EAGLView *_glView;
	//Interface elements
	UIButton *_saveButton;
	UIButton *_sendMailButton;
	UILabel *_descriptionLabel;
	UISlider *_xRotationSlider;
	UISlider *_yRotationSlider;
	UISlider *_zRotationSlider;
	UISlider *_zTranslationSlider;
	UISlider *_scaleFactorSlider;
	UITextField *_xRotationTextField;
	UITextField *_yRotationTextField;
	UITextField *_zRotationTextField;
	UITextField *_zTranslationTextField;
	UITextField *_scaleFactorTextField;
}

/*! \n */
@property (nonatomic, assign) id<ParametersDelegate> delegate;
/*! 
	@brief The EAGLView instance the controller is modifying the displaay.
 */
@property (nonatomic, assign) EAGLView *glView;
/*! \n*/
@property (nonatomic, retain) IBOutlet UIButton *saveButton;
/*! \n*/
@property (nonatomic, retain) IBOutlet UIButton *sendMailButton;
/*! \n*/
@property (nonatomic, retain) IBOutlet UILabel *descriptionLabel;
/*! \n*/
@property (nonatomic, retain) IBOutlet UISlider *xRotationSlider;
/*! \n*/
@property (nonatomic, retain) IBOutlet UISlider *yRotationSlider;
/*! \n*/
@property (nonatomic, retain) IBOutlet UISlider *zRotationSlider;
/*! \n*/
@property (nonatomic, retain) IBOutlet UISlider *zTranslationSlider;
/*! \n*/
@property (nonatomic, retain) IBOutlet UISlider *scaleFactorSlider;
/*! \n*/
@property (nonatomic, retain) IBOutlet UITextField *xRotationTextField;
/*! \n*/
@property (nonatomic, retain) IBOutlet UITextField *yRotationTextField;
/*! \n*/
@property (nonatomic, retain) IBOutlet UITextField *zRotationTextField;
/*! \n*/
@property (nonatomic, retain) IBOutlet UITextField *zTranslationTextField;
/*! \n*/
@property (nonatomic, retain) IBOutlet UITextField *scaleFactorTextField;


/*!
	@brief	Refresh the view according to the parameters of the Object3D instance hold
*/
- (void)refreshView;

/*!
 @brief	This method saves the settings by serializing the object and storing the xml file in the document directory
 */
- (IBAction)save:(id)sender;
/*!
 @brief	This method send the xml file with the right settings by mail.
 
 Using this method you can retrieve the xml file and add it in a client app for instance where you don't want the 
 user to be able to display the parametersView.
 */
- (IBAction)sendByMail:(id)sender;

/*! \n*/
- (IBAction)xRotationSliderValueChanged:(id)sender;
/*! \n*/
- (IBAction)yRotationSliderValueChanged:(id)sender;
/*! \n*/
- (IBAction)zRotationSliderValueChanged:(id)sender;
/*! \n*/
- (IBAction)zTranslationSliderValueChanged:(id)sender;
/*! \n*/
- (IBAction)scaleFactorSliderValueChanged:(id)sender;
/*! \n*/
- (IBAction)xRotationTextFieldValueChanged:(id)sender;
/*! \n*/
- (IBAction)yRotationTextFieldValueChanged:(id)sender;
/*! \n*/
- (IBAction)zRotationTextFieldValueChanged:(id)sender;
/*! \n*/
- (IBAction)zTranslationTextFieldValueChanged:(id)sender;
/*! \n*/
- (IBAction)scaleFactorTextFieldValueChanged:(id)sender;

@end
