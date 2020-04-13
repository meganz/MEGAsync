//
//  NSPopover+MISSINGBackgroundView.h
//  NSPopover+MISSINGBackgroundView
//
//  Created by Valentin Shergin on 20/05/15.
//  Copyright (c) 2015 Shergin Research. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSPopover (MISSINGBackgroundView)

@property (nonatomic, readonly) NSView *backgroundView;
@property (nonatomic) NSColor *backgroundColor;

@end
