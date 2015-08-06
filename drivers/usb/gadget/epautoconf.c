/*
 * epautoconf.c -- endpoint autoconfiguration for usb gadget drivers
 *
 * Copyright (C) 2004 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/device.h>

#include <linux/ctype.h>
#include <linux/string.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include "gadget_chips.h"

static struct usb_ep *
find_ep (struct usb_gadget *gadget, const char *name)
{
	struct usb_ep	*ep;

	list_for_each_entry (ep, &gadget->ep_list, ep_list) {
		if (0 == strcmp (ep->name, name))
			return ep;
	}
	return NULL;
}

/**
 * usb_ep_autoconfig_ss() - choose an endpoint matching the ep
 * descriptor and ep companion descriptor
 * @gadget: The device to which the endpoint must belong.
 * @desc: Endpoint descriptor, with endpoint direction and transfer mode
 *    initialized.  For periodic transfers, the maximum packet
 *    size must also be initialized.  This is modified on
 *    success.
 * @ep_comp: Endpoint companion descriptor, with the required
 *    number of streams. Will be modified when the chosen EP
 *    supports a different number of streams.
 *
 * This routine replaces the usb_ep_autoconfig when needed
 * superspeed enhancments. If such enhancemnets are required,
 * the FD should call usb_ep_autoconfig_ss directly and provide
 * the additional ep_comp parameter.
 *
 * By choosing an endpoint to use with the specified descriptor,
 * this routine simplifies writing gadget drivers that work with
 * multiple USB device controllers.  The endpoint would be
 * passed later to usb_ep_enable(), along with some descriptor.
 *
 * That second descriptor won't always be the same as the first one.
 * For example, isochronous endpoints can be autoconfigured for high
 * bandwidth, and then used in several lower bandwidth altsettings.
 * Also, high and full speed descriptors will be different.
 *
 * Be sure to examine and test the results of autoconfiguration
 * on your hardware.  This code may not make the best choices
 * about how to use the USB controller, and it can't know all
 * the restrictions that may apply. Some combinations of driver
 * and hardware won't be able to autoconfigure.
 *
 * On success, this returns an un-claimed usb_ep, and modifies the endpoint
 * descriptor bEndpointAddress.  For bulk endpoints, the wMaxPacket value
 * is initialized as if the endpoint were used at full speed and
 * the bmAttribute field in the ep companion descriptor is
 * updated with the assigned number of streams if it is
 * different from the original value. To prevent the endpoint
 * from being returned by a later autoconfig call, claim it by
 * assigning ep->claimed to true.
 *
 * On failure, this returns a null endpoint descriptor.
 */
struct usb_ep *usb_ep_autoconfig_ss(
	struct usb_gadget		*gadget,
	struct usb_endpoint_descriptor	*desc,
	struct usb_ss_ep_comp_descriptor *ep_comp
)
{
	struct usb_ep	*ep;
	u8		type;

	type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;

	if (gadget->ops->match_ep) {
		ep = gadget->ops->match_ep(gadget, desc, ep_comp);
		if (ep)
			goto found_ep;
	}

	/* First, apply chip-specific "best usage" knowledge.
	 * This might make a good usb_gadget_ops hook ...
	 */
	if (gadget_is_net2280(gadget)) {
		char name[8];

		if (type == USB_ENDPOINT_XFER_INT) {
			/* ep-e, ep-f are PIO with only 64 byte fifos */
			ep = find_ep(gadget, "ep-e");
			if (ep && usb_gadget_ep_match_desc(gadget,
					ep, desc, ep_comp))
				goto found_ep;
			ep = find_ep(gadget, "ep-f");
			if (ep && usb_gadget_ep_match_desc(gadget,
					ep, desc, ep_comp))
				goto found_ep;
		}

		/* USB3380: use same address for usb and hardware endpoints */
		snprintf(name, sizeof(name), "ep%d%s", usb_endpoint_num(desc),
				usb_endpoint_dir_in(desc) ? "in" : "out");
		ep = find_ep(gadget, name);
		if (ep && usb_gadget_ep_match_desc(gadget, ep, desc, ep_comp))
			goto found_ep;
	} else if (gadget_is_goku (gadget)) {
		if (USB_ENDPOINT_XFER_INT == type) {
			/* single buffering is enough */
			ep = find_ep(gadget, "ep3-bulk");
			if (ep && usb_gadget_ep_match_desc(gadget,
					ep, desc, ep_comp))
				goto found_ep;
		} else if (USB_ENDPOINT_XFER_BULK == type
				&& (USB_DIR_IN & desc->bEndpointAddress)) {
			/* DMA may be available */
			ep = find_ep(gadget, "ep2-bulk");
			if (ep && usb_gadget_ep_match_desc(gadget,
					ep, desc, ep_comp))
				goto found_ep;
		}

#ifdef CONFIG_BLACKFIN
	} else if (gadget_is_musbhdrc(gadget)) {
		if ((USB_ENDPOINT_XFER_BULK == type) ||
		    (USB_ENDPOINT_XFER_ISOC == type)) {
			if (USB_DIR_IN & desc->bEndpointAddress)
				ep = find_ep (gadget, "ep5in");
			else
				ep = find_ep (gadget, "ep6out");
		} else if (USB_ENDPOINT_XFER_INT == type) {
			if (USB_DIR_IN & desc->bEndpointAddress)
				ep = find_ep(gadget, "ep1in");
			else
				ep = find_ep(gadget, "ep2out");
		} else
			ep = NULL;
		if (ep && usb_gadget_ep_match_desc(gadget, ep, desc, ep_comp))
			goto found_ep;
#endif
	}

	/* Second, look at endpoints until an unclaimed one looks usable */
	list_for_each_entry (ep, &gadget->ep_list, ep_list) {
		if (usb_gadget_ep_match_desc(gadget, ep, desc, ep_comp))
			goto found_ep;
	}

	/* Fail */
	return NULL;
found_ep:

	/*
	 * If the protocol driver hasn't yet decided on wMaxPacketSize
	 * and wants to know the maximum possible, provide the info.
	 */
	if (desc->wMaxPacketSize == 0)
		desc->wMaxPacketSize = cpu_to_le16(ep->maxpacket_limit);

	/* report address */
	desc->bEndpointAddress &= USB_DIR_IN;
	if (isdigit(ep->name[2])) {
		u8 num = simple_strtoul(&ep->name[2], NULL, 10);
		desc->bEndpointAddress |= num;
	} else if (desc->bEndpointAddress & USB_DIR_IN) {
		if (++gadget->in_epnum > 15)
			return NULL;
		desc->bEndpointAddress = USB_DIR_IN | gadget->in_epnum;
	} else {
		if (++gadget->out_epnum > 15)
			return NULL;
		desc->bEndpointAddress |= gadget->out_epnum;
	}

	/* report (variable) full speed bulk maxpacket */
	if ((type == USB_ENDPOINT_XFER_BULK) && !ep_comp) {
		int size = ep->maxpacket_limit;

		/* min() doesn't work on bitfields with gcc-3.5 */
		if (size > 64)
			size = 64;
		desc->wMaxPacketSize = cpu_to_le16(size);
	}

	ep->address = desc->bEndpointAddress;
	ep->desc = NULL;
	ep->comp_desc = NULL;
	ep->claimed = true;
	return ep;
}
EXPORT_SYMBOL_GPL(usb_ep_autoconfig_ss);

/**
 * usb_ep_autoconfig() - choose an endpoint matching the
 * descriptor
 * @gadget: The device to which the endpoint must belong.
 * @desc: Endpoint descriptor, with endpoint direction and transfer mode
 *	initialized.  For periodic transfers, the maximum packet
 *	size must also be initialized.  This is modified on success.
 *
 * By choosing an endpoint to use with the specified descriptor, this
 * routine simplifies writing gadget drivers that work with multiple
 * USB device controllers.  The endpoint would be passed later to
 * usb_ep_enable(), along with some descriptor.
 *
 * That second descriptor won't always be the same as the first one.
 * For example, isochronous endpoints can be autoconfigured for high
 * bandwidth, and then used in several lower bandwidth altsettings.
 * Also, high and full speed descriptors will be different.
 *
 * Be sure to examine and test the results of autoconfiguration on your
 * hardware.  This code may not make the best choices about how to use the
 * USB controller, and it can't know all the restrictions that may apply.
 * Some combinations of driver and hardware won't be able to autoconfigure.
 *
 * On success, this returns an un-claimed usb_ep, and modifies the endpoint
 * descriptor bEndpointAddress.  For bulk endpoints, the wMaxPacket value
 * is initialized as if the endpoint were used at full speed.  To prevent
 * the endpoint from being returned by a later autoconfig call, claim it
 * by assigning ep->claimed to true.
 *
 * On failure, this returns a null endpoint descriptor.
 */
struct usb_ep *usb_ep_autoconfig(
	struct usb_gadget		*gadget,
	struct usb_endpoint_descriptor	*desc
)
{
	return usb_ep_autoconfig_ss(gadget, desc, NULL);
}
EXPORT_SYMBOL_GPL(usb_ep_autoconfig);

/**
 * usb_ep_autoconfig_reset - reset endpoint autoconfig state
 * @gadget: device for which autoconfig state will be reset
 *
 * Use this for devices where one configuration may need to assign
 * endpoint resources very differently from the next one.  It clears
 * state such as ep->claimed and the record of assigned endpoints
 * used by usb_ep_autoconfig().
 */
void usb_ep_autoconfig_reset (struct usb_gadget *gadget)
{
	struct usb_ep	*ep;

	list_for_each_entry (ep, &gadget->ep_list, ep_list) {
		ep->claimed = false;
	}
	gadget->in_epnum = 0;
	gadget->out_epnum = 0;
}
EXPORT_SYMBOL_GPL(usb_ep_autoconfig_reset);
