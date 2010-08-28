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

#import "Object3DParametersViewController.h"
#import "EAGLView.h"
#import "Object3D.h"

/*!
	Private interface for  the Object3DParametersViewController class.
 */
@interface Object3DParametersViewController () 

/*!
 @brief	Refresh the values of the sliders and the textfields
 */
- (void)refreshView;

/*!
	@brief	Enable the buttons, sliders and textfields
*/
- (void)enableUI;
/*!
	@brief	Disable the buttons, sliders and textfields
 */
- (void)disableUI;

@end

@implementation Object3DParametersViewController

@synthesize delegate  =_delegate;
@synthesize glView = _glView;
@synthesize saveButton = _saveButton;
@synthesize sendMailButton = _sendMailButton;
@synthesize descriptionLabel = _descriptionLabel;
@synthesize xRotationSlider = _xRotationSlider;
@synthesize yRotationSlider = _yRotationSlider;
@synthesize zRotationSlider = _zRotationSlider;
@synthesize zTranslationSlider = _zTranslationSlider;
@synthesize scaleFactorSlider = _scaleFactorSlider;
@synthesize xRotationTextField = _xRotationTextField;
@synthesize yRotationTextField = _yRotationTextField;
@synthesize zRotationTextField = _zRotationTextField;
@synthesize zTranslationTextField = _zTranslationTextField;
@synthesize scaleFactorTextField = _scaleFactorTextField;

#pragma mark -
#pragma mark Initialization

- (void)viewDidLoad {
	[self refreshView];
}

#pragma mark -
#pragma mark Display

- (void)refreshView {
	if (self.glView.markerID == -1) {
		self.descriptionLabel.text = @"No markers detected";
		[self disableUI];
	}
	else if (!self.glView.object) {
		self.descriptionLabel.text = [NSString stringWithFormat:@"No object associated to marker number %d", self.glView.markerID];
		[self disableUI];
	}
	else {
		[self enableUI];
		NSLog(@"%@",self.glView.object.name);
		self.descriptionLabel.text = [NSString stringWithFormat:@"Object named %@ and associated to marker number %d",
									  self.glView.object.name, self.glView.markerID];
		NSLog(@"%@",self.glView.object.name);
		self.xRotationSlider.value = self.glView.object.xRotation;
		self.yRotationSlider.value = self.glView.object.yRotation;
		self.zRotationSlider.value = self.glView.object.zRotation;
		self.zTranslationSlider.value = self.glView.object.zTranslation;
		self.scaleFactorSlider.value = self.glView.object.scaleFactor;
		self.xRotationTextField.text = [NSString stringWithFormat:@"%.2f", self.glView.object.xRotation];
		self.yRotationTextField.text = [NSString stringWithFormat:@"%.2f", self.glView.object.yRotation];
		self.zRotationTextField.text = [NSString stringWithFormat:@"%.2f", self.glView.object.zRotation];
		self.zTranslationTextField.text = [NSString stringWithFormat:@"%.2f", self.glView.object.zTranslation];
		self.scaleFactorTextField.text = [NSString stringWithFormat:@"%.2f", self.glView.object.scaleFactor];
		
		[self.glView drawView];
	}
}

- (void)disableUI {
	self.saveButton.enabled = NO;
	self.sendMailButton.enabled = NO;
	self.xRotationSlider.enabled = NO;
	self.yRotationSlider.enabled = NO;
	self.zRotationSlider.enabled = NO;
	self.zTranslationSlider.enabled = NO;
	self.scaleFactorSlider.enabled = NO;
	self.xRotationTextField.enabled = NO;
	self.yRotationTextField.enabled = NO;
	self.zRotationTextField.enabled = NO;
	self.zTranslationTextField.enabled = NO;
	self.scaleFactorTextField.enabled = NO;
}

- (void)enableUI {
	self.saveButton.enabled = YES;
	self.sendMailButton.enabled = YES;
	self.xRotationSlider.enabled = YES;
	self.yRotationSlider.enabled = YES;
	self.zRotationSlider.enabled = YES;
	self.zTranslationSlider.enabled = YES;
	self.scaleFactorSlider.enabled = YES;
	self.xRotationTextField.enabled = YES;
	self.yRotationTextField.enabled = YES;
	self.zRotationTextField.enabled = YES;
	self.zTranslationTextField.enabled = YES;
	self.scaleFactorTextField.enabled = YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
	[textField resignFirstResponder];
	return YES;
}

#pragma mark -
#pragma mark Object modifying
- (IBAction)xRotationSliderValueChanged:(id)sender {
	self.glView.object.xRotation = self.xRotationSlider.value;
	[self refreshView];
}
- (IBAction)yRotationSliderValueChanged:(id)sender {
	self.glView.object.yRotation = self.yRotationSlider.value;
	[self refreshView];
}
- (IBAction)zRotationSliderValueChanged:(id)sender {
	self.glView.object.zRotation = self.zRotationSlider.value;
	[self refreshView];
}
- (IBAction)zTranslationSliderValueChanged:(id)sender {
	self.glView.object.zTranslation = self.zTranslationSlider.value;
	[self refreshView];
}
- (IBAction)scaleFactorSliderValueChanged:(id)sender {
	self.glView.object.scaleFactor = self.scaleFactorSlider.value;
	[self refreshView];
}

- (IBAction)xRotationTextFieldValueChanged:(id)sender {
	self.glView.object.xRotation = [self.xRotationTextField.text floatValue];
	[self refreshView];
}

- (IBAction)yRotationTextFieldValueChanged:(id)sender {
	self.glView.object.yRotation = [self.yRotationTextField.text floatValue];
	[self refreshView];
}

- (IBAction)zRotationTextFieldValueChanged:(id)sender {
	self.glView.object.zRotation = [self.zRotationTextField.text floatValue];
	[self refreshView];
}

- (IBAction)zTranslationTextFieldValueChanged:(id)sender {
	self.glView.object.zTranslation = [self.zTranslationTextField.text floatValue];
	[self refreshView];
}

- (IBAction)scaleFactorTextFieldValueChanged:(id)sender {
	self.glView.object.scaleFactor = [self.scaleFactorTextField.text floatValue];
	[self refreshView];
}

#pragma mark -
#pragma mark save/mail

- (IBAction)save:(id)sender {
	[self.glView.object writeInDocumentDirectory];
}

- (NSString *)applicationDocumentsDirectory {
	
    NSArray *paths = 
	NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
    return basePath;
}

- (IBAction)sendByMail:(id)sender {
	[self save:self];
	if ([MFMailComposeViewController canSendMail]) {
		MFMailComposeViewController* mailController = [[MFMailComposeViewController alloc] init];
		mailController.mailComposeDelegate = self;
		[mailController setSubject:@"Augmented reality"];
		[mailController setMessageBody:@"Hello there." isHTML:NO];
		[mailController addAttachmentData:[NSData dataWithContentsOfFile:self.glView.object.xmlFilePath] 
								 mimeType:@"text/xml" fileName:[self.glView.object.xmlFilePath lastPathComponent]];
		[self.delegate object3DParametersViewControllerWillDisplayModalViewController:self];
		[self presentModalViewController:mailController animated:YES];
		[mailController release];
		
	}
	else {
		NSLog(@"The device can't send mail");
	}

}
									   
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error {
	[self.delegate object3DParametersViewControllerWillDismissModalViewController:self];
	[self dismissModalViewControllerAnimated:YES];
}									   

#pragma mark -
#pragma mark Memory managment
- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    self.xRotationSlider = nil;
	self.yRotationSlider = nil;
    self.zRotationSlider = nil;
	self.scaleFactorSlider = nil;
	self.xRotationTextField = nil;
	self.yRotationTextField = nil;
	self.zRotationTextField = nil;
	self.scaleFactorTextField = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
