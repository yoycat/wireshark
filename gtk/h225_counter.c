/* h225_counter.c
 * h225 message counter for ethereal
 * Copyright 2003 Lars Roland
 *
 * $Id: h225_counter.c,v 1.12 2004/02/13 00:53:35 guy Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#include <string.h>

#include <gtk/gtk.h>

#include <epan/packet_info.h>
#include <epan/epan.h>
#include <epan/value_string.h>

#include "tap_menu.h"
#include "../tap.h"
#include "../register.h"
#include "../packet-h225.h"
#include "gtk_stat_util.h"
#include "compat_macros.h"
#include "../simple_dialog.h"
#include "dlg_utils.h"
#include "../file.h"
#include "../globals.h"
#include "../tap_dfilter_dlg.h"
#include "tap_dfilter_dlg.h"
#include "ui_util.h"


extern GtkWidget *main_display_filter_widget;

void gtk_h225counter_init(char *optarg);

tap_dfilter_dlg h225_counter_dlg = {"H.225 Messages and Message Reasons", "h225,counter", gtk_h225counter_init, -1};

/* following values represent the size of their valuestring arrays */

#define RAS_MSG_TYPES 33
#define CS_MSG_TYPES 13

#define GRJ_REASONS 8
#define RRJ_REASONS 18
#define URQ_REASONS 6
#define URJ_REASONS 6
#define ARJ_REASONS 22
#define BRJ_REASONS 8
#define DRQ_REASONS 3
#define DRJ_REASONS 4
#define LRJ_REASONS 16
#define IRQNAK_REASONS 4
#define REL_CMP_REASONS 26
#define FACILITY_REASONS 11


/* used to keep track of the statistics for an entire program interface */
typedef struct _h225counter_t {
	GtkWidget *win;
	GtkWidget *vbox;
	char *filter;
	GtkWidget *scrolled_window;
	GtkCList *table;
	guint32 ras_msg[RAS_MSG_TYPES + 1];
        guint32 cs_msg[CS_MSG_TYPES + 1];
        guint32 grj_reason[GRJ_REASONS + 1];
        guint32 rrj_reason[RRJ_REASONS + 1];
        guint32 urq_reason[URQ_REASONS + 1];
        guint32 urj_reason[URJ_REASONS + 1];
        guint32 arj_reason[ARJ_REASONS + 1];
        guint32 brj_reason[BRJ_REASONS + 1];
        guint32 drq_reason[DRQ_REASONS + 1];
        guint32 drj_reason[DRJ_REASONS + 1];
        guint32 lrj_reason[LRJ_REASONS + 1];
        guint32 irqnak_reason[IRQNAK_REASONS + 1];
        guint32 rel_cmp_reason[REL_CMP_REASONS + 1];
        guint32 facility_reason[FACILITY_REASONS + 1];
} h225counter_t;


static void
h225counter_reset(void *phs)
{
	h225counter_t *hs=(h225counter_t *)phs;
	int i;

	for(i=0;i<=RAS_MSG_TYPES;i++) {
		hs->ras_msg[i] = 0;
	}
	for(i=0;i<=CS_MSG_TYPES;i++) {
		hs->cs_msg[i] = 0;
	}
	for(i=0;i<=GRJ_REASONS;i++) {
		hs->grj_reason[i] = 0;
	}
	for(i=0;i<=RRJ_REASONS;i++) {
		hs->rrj_reason[i] = 0;
	}
	for(i=0;i<=URQ_REASONS;i++) {
		hs->urq_reason[i] = 0;
	}
	for(i=0;i<=URJ_REASONS;i++) {
		hs->urj_reason[i] = 0;
	}
	for(i=0;i<=ARJ_REASONS;i++) {
		hs->arj_reason[i] = 0;
	}
	for(i=0;i<=BRJ_REASONS;i++) {
		hs->brj_reason[i] = 0;
	}
	for(i=0;i<=DRQ_REASONS;i++) {
		hs->drq_reason[i] = 0;
	}
	for(i=0;i<=DRJ_REASONS;i++) {
		hs->drj_reason[i] = 0;
	}
	for(i=0;i<=LRJ_REASONS;i++) {
		hs->lrj_reason[i] = 0;
	}
	for(i=0;i<=IRQNAK_REASONS;i++) {
		hs->irqnak_reason[i] = 0;
	}
	for(i=0;i<=REL_CMP_REASONS;i++) {
		hs->rel_cmp_reason[i] = 0;
	}
	for(i=0;i<=FACILITY_REASONS;i++) {
		hs->facility_reason[i] = 0;
	}
}

static int
h225counter_packet(void *phs, packet_info *pinfo _U_, epan_dissect_t *edt _U_, void *phi)
{
	h225counter_t *hs=(h225counter_t *)phs;
	h225_packet_info *pi=phi;

	switch (pi->msg_type) {

	case H225_RAS:
		if(pi->msg_tag==-1) { /* uninitialized */
			return 0;
		}
		else if (pi->msg_tag >= RAS_MSG_TYPES) { /* unknown */
			hs->ras_msg[RAS_MSG_TYPES]++;
		}
		else {
			hs->ras_msg[pi->msg_tag]++;
		}

		/* Look for reason tag */
		if(pi->reason==-1) { /* uninitialized */
			break;
		}

		switch(pi->msg_tag) {

		case 2:	/* GRJ */
			if(pi->reason < GRJ_REASONS)
				hs->grj_reason[pi->reason]++;
			else
				hs->grj_reason[GRJ_REASONS]++;
			break;
		case 5:	/* RRJ */
			if(pi->reason < RRJ_REASONS)
				hs->rrj_reason[pi->reason]++;
			else
				hs->rrj_reason[RRJ_REASONS]++;
			break;
		case 6:	/* URQ */
			if(pi->reason < URQ_REASONS)
				hs->urq_reason[pi->reason]++;
			else
				hs->urq_reason[URQ_REASONS]++;
			break;
		case 8:	/* URJ */
			if(pi->reason < URJ_REASONS)
				hs->urj_reason[pi->reason]++;
			else
				hs->urj_reason[URJ_REASONS]++;
			break;
		case 11: /* ARJ */
			if(pi->reason < ARJ_REASONS)
				hs->arj_reason[pi->reason]++;
			else
				hs->arj_reason[ARJ_REASONS]++;
			break;
		case 14: /* BRJ */
			if(pi->reason < BRJ_REASONS)
				hs->brj_reason[pi->reason]++;
			else
				hs->brj_reason[BRJ_REASONS]++;
			break;
		case 15: /* DRQ */
			if(pi->reason < DRQ_REASONS)
				hs->drq_reason[pi->reason]++;
			else
				hs->drq_reason[DRQ_REASONS]++;
			break;
		case 17: /* DRJ */
			if(pi->reason < DRJ_REASONS)
				hs->drj_reason[pi->reason]++;
			else
				hs->drj_reason[DRJ_REASONS]++;
			break;
		case 20: /* LRJ */
			if(pi->reason < LRJ_REASONS)
				hs->lrj_reason[pi->reason]++;
			else
				hs->lrj_reason[LRJ_REASONS]++;
			break;
		case 29: /* IRQ Nak */
			if(pi->reason < IRQNAK_REASONS)
				hs->irqnak_reason[pi->reason]++;
			else
				hs->irqnak_reason[IRQNAK_REASONS]++;
			break;

		default:
			/* do nothing */
			break;
		}

		break;

	case H225_CS:
		if(pi->msg_tag==-1) { /* uninitialized */
			return 0;
		}
		else if (pi->msg_tag >= CS_MSG_TYPES) { /* unknown */
			hs->cs_msg[CS_MSG_TYPES]++;
		}
		else {
			hs->cs_msg[pi->msg_tag]++;
		}

		/* Look for reason tag */
		if(pi->reason==-1) { /* uninitialized */
			break;
		}

		switch(pi->msg_tag) {

		case 5:	/* ReleaseComplete */
			if(pi->reason < REL_CMP_REASONS)
				hs->rel_cmp_reason[pi->reason]++;
			else
				hs->rel_cmp_reason[REL_CMP_REASONS]++;
			break;
		case 6:	/* Facility */
			if(pi->reason < FACILITY_REASONS)
				hs->facility_reason[pi->reason]++;
			else
				hs->facility_reason[FACILITY_REASONS]++;
			break;
		default:
			/* do nothing */
			break;
		}

		break;

	default:
		return 0;
		break;
	}

	return 1;
}

static void
h225counter_draw(void *phs)
{
	h225counter_t *hs=(h225counter_t *)phs;
	int i,j;
	char *str[2];

	for(i=0;i<2;i++) {
		str[i]=g_malloc(sizeof(char[256]));
	}
	/* Now print Message and Reason Counter Table */
	/* clear list before printing */
	gtk_clist_clear(hs->table);

	for(i=0;i<=RAS_MSG_TYPES;i++) {
		if(hs->ras_msg[i]!=0) {
			sprintf(str[0], "%s", val_to_str(i,RasMessage_vals,"unknown ras-messages  "));
			sprintf(str[1], "%d", hs->ras_msg[i]);
			gtk_clist_append(hs->table, str);

			/* reason counter */
			switch(i) {
			case 2: /* GRJ */
				for(j=0;j<=GRJ_REASONS;j++) {
					if(hs->grj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,GatekeeperRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->grj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 5: /* RRJ */
				for(j=0;j<=RRJ_REASONS;j++) {
					if(hs->rrj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,RegistrationRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->rrj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 6: /* URQ */
				for(j=0;j<=URQ_REASONS;j++) {
					if(hs->urq_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,UnregRequestReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->urq_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 8: /* URJ */
				for(j=0;j<=URJ_REASONS;j++) {
					if(hs->urj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,UnregRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->urj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 11: /* ARJ */
				for(j=0;j<=ARJ_REASONS;j++) {
					if(hs->arj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,AdmissionRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->arj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 14: /* BRJ */
				for(j=0;j<=BRJ_REASONS;j++) {
					if(hs->brj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,BandRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->brj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 15: /* DRQ */
				for(j=0;j<=DRQ_REASONS;j++) {
					if(hs->drq_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,DisengageReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->drq_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 17: /* DRJ */
				for(j=0;j<=DRJ_REASONS;j++) {
					if(hs->drj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,DisengageRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->drj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 20: /* LRJ */
				for(j=0;j<=LRJ_REASONS;j++) {
					if(hs->lrj_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,LocationRejectReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->lrj_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 29: /* IRQNak */
				for(j=0;j<=IRQNAK_REASONS;j++) {
					if(hs->irqnak_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,InfoRequestNakReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->irqnak_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			default:
				break;
			}
			/* end of reason counter*/
		}
	}

	for(i=0;i<=CS_MSG_TYPES;i++) {
		if(hs->cs_msg[i]!=0) {
			sprintf(str[0], "%s", val_to_str(i,h323_message_body_vals,"unknown cs-messages   "));
			sprintf(str[1], "%d", hs->cs_msg[i]);
			gtk_clist_append(hs->table, str);

			/* reason counter */
			switch(i) {
			case 5: /* ReleaseComplete */
				for(j=0;j<=REL_CMP_REASONS;j++) {
					if(hs->rel_cmp_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,ReleaseCompleteReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->rel_cmp_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			case 6: /* Facility */
				for(j=0;j<=FACILITY_REASONS;j++) {
					if(hs->facility_reason[j]!=0) {
						sprintf(str[0], "    %s", val_to_str(j,FacilityReason_vals,"unknown reason   "));
						sprintf(str[1], "%d", hs->facility_reason[j]);
						gtk_clist_append(hs->table, str);
					}
				}
				break;
			default:
				break;
			}
		}
	}

	gtk_widget_show(GTK_WIDGET(hs->table));

}

void protect_thread_critical_region(void);
void unprotect_thread_critical_region(void);
static void
win_destroy_cb(GtkWindow *win _U_, gpointer data)
{
	h225counter_t *hs=(h225counter_t *)data;

	protect_thread_critical_region();
	remove_tap_listener(hs);
	unprotect_thread_critical_region();

	if(hs->filter){
		g_free(hs->filter);
		hs->filter=NULL;
	}
	g_free(hs);
}


static gchar *titles[]={"Message Type or Reason",
			"Count" };

void
gtk_h225counter_init(char *optarg)
{
	h225counter_t *hs;
	char *filter=NULL;
	GString *error_string;

	if(strncmp(optarg,"h225,counter,",13) == 0){
		filter=optarg+13;
	} else {
		filter=g_malloc(1);
		*filter='\0';
	}

	hs=g_malloc(sizeof(h225counter_t));
	hs->filter=g_malloc(strlen(filter)+1);
	strcpy(hs->filter, filter);

	h225counter_reset(hs);

	hs->win=window_new(GTK_WINDOW_TOPLEVEL, NULL);
	SIGNAL_CONNECT(hs->win, "destroy", win_destroy_cb, hs);

	hs->vbox=gtk_vbox_new(FALSE, 0);

	init_main_stat_window(hs->win, hs->vbox, "ITU-T H.225 Message and Message Reason Counter", filter);

        /* init a scrolled window*/
	hs->scrolled_window = scrolled_window_new(NULL, NULL);
	WIDGET_SET_SIZE(hs->scrolled_window, 400, 200);

	hs->table = create_stat_table(hs->scrolled_window, hs->vbox, 2, titles);

	error_string=register_tap_listener("h225", hs, filter, h225counter_reset, h225counter_packet, h225counter_draw);
	if(error_string){
		simple_dialog(ESD_TYPE_ERROR, ESD_BTN_OK, error_string->str);
		g_string_free(error_string, TRUE);
		g_free(hs->filter);
		g_free(hs);
		return;
	}

	gtk_widget_show_all(hs->win);
	redissect_packets(&cfile);
}

void
register_tap_listener_gtk_h225counter(void)
{
	register_ethereal_tap("h225,counter", gtk_h225counter_init);
}

void
register_tap_menu_gtk_h225counter(void)
{
	register_tap_menu_item("_Statistics/Watch protocol/ITU-T H.225...",
	    gtk_tap_dfilter_dlg_cb, NULL, NULL, &(h225_counter_dlg));
}
