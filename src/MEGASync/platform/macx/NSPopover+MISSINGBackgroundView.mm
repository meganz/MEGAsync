//
//  NSPopover+MISSINGBackgroundView.m
//  NSPopover+MISSINGBackgroundView
//
//  Created by Valentin Shergin on 20/05/15.
//  Copyright (c) 2015 Shergin Research. All rights reserved.
//


#import "NSPopover+MISSINGBackgroundView.h"

#import <objc/runtime.h>

@implementation NSPopover (MISSINGBackgroundView)

- (NSView *)backgroundView
{
	NSView *backgroundView = objc_getAssociatedObject(self, @selector(backgroundView));

	if (!backgroundView) {
		backgroundView = [[NSView alloc] init];
		objc_setAssociatedObject(self, @selector(backgroundView), backgroundView, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(popoverWillOpen:) name:NSPopoverWillShowNotification object:self];
	}

	return backgroundView;
}

- (void)popoverWillOpen:(NSNotification *)notification
{
	NSView *backgroundView = self.backgroundView;
	if (!backgroundView.superview) {
		NSView *contentView = self.contentViewController.view;
		NSView *frameView = [contentView superview];
		frameView.wantsLayer = YES;
		backgroundView.frame = NSInsetRect(frameView.frame, 1.f, 1.f);
		backgroundView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
		[frameView addSubview:backgroundView positioned:NSWindowBelow relativeTo:contentView];
	}
}

- (void)setBackgroundColor:(NSColor *)backgroundColor
{
	NSView *backgroundView = self.backgroundView;
	NSLog(@"setBackgroundColor: popover: %@, backgroundView: %@, color: %@", self, self.backgroundView, backgroundColor);
	backgroundView.wantsLayer = YES;
	backgroundView.layer.backgroundColor = [backgroundColor CGColor];
}

- (NSColor *)backgroundColor
{
	NSLog(@"backgroundColor: popover: %@, backgroundView: %@", self, self.backgroundView);
	CGColorRef backgroundColor = self.backgroundView.layer.backgroundColor;
	return backgroundColor ? [NSColor colorWithCGColor:backgroundColor] : nil;
}

@end
