

/*
 * client_example2.c
 *
 * This example shows how to browse the data model of an unknown device and display it in an interface.
 */

#include "iec61850_client.h"
#include <stdlib.h>
#include <stdio.h>
#define GTK_ENABLE_BROKEN
//include <config.h>
#include <gtk/gtk.h>

/* for all the GtkItem:: and GtkTreeItem:: signals */
static void cb_itemsignal( GtkWidget *item, gchar *signame )
{
	gchar *name;
	GtkLabel *label;

	/* It's a Bin, so it has one child, which we know to be a label, so get that */
	label = GTK_LABEL (GTK_BIN (item)->child);
	/* Get the text of the label */
	gtk_label_get (label, &name);
	/* Get the level of the tree which the item is in */
	g_print ("%s called for item %s->%p, level %d\n", signame, name,item, GTK_TREE (item->parent)->level);
}

/* Note that this is never called */
static void cb_unselect_child( GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree )
{
	g_print ("unselect_child called for root tree %p, subtree %p, child %p\n", root_tree, subtree, child);
}

/* Note that this is called every time the user clicks on an item,
   whether it is already selected or not. */
static void cb_select_child (GtkWidget *root_tree, GtkWidget *child, GtkWidget *subtree)
{
	g_print ("select_child called for root tree %p, subtree %p, child %p\n", root_tree, subtree, child);
}

static void cb_selection_changed( GtkWidget *tree )
{
	GList *i;
  
	g_print ("selection_change called for tree %p\n", tree);
	g_print ("selected objects are:\n");

	i = GTK_TREE_SELECTION_OLD (tree);
	while (i) 
	{
		gchar *name;
		GtkLabel *label;
		GtkWidget *item;

		/* Get a GtkWidget pointer from the list node */
		item = GTK_WIDGET (i->data);
		label = GTK_LABEL (GTK_BIN (item)->child);
		gtk_label_get (label, &name);
		g_print ("\t%s on level %d\n", name, GTK_TREE(item->parent)->level);
		i = i->next;
	}
}

void create_item(GtkWidget **tree,  GtkWidget **item, gchar **label)
{
	/* Create a tree item */
    *item = gtk_tree_item_new_with_label (*label);
	
    /* Connect all GtkItem:: and GtkTreeItem:: signals */
    g_signal_connect (G_OBJECT (*item), "select"  ,G_CALLBACK (cb_itemsignal), "select");
    g_signal_connect (G_OBJECT (*item), "deselect",G_CALLBACK (cb_itemsignal), "deselect");
    g_signal_connect (G_OBJECT (*item), "toggle"  ,G_CALLBACK (cb_itemsignal), "toggle");
    g_signal_connect (G_OBJECT (*item), "expand"  ,G_CALLBACK (cb_itemsignal), "expand");
    g_signal_connect (G_OBJECT (*item), "collapse",G_CALLBACK (cb_itemsignal), "collapse");
    /* Show it - this can be done at any time */
   	gtk_tree_append(GTK_TREE (*tree), *item);
   	/* Show it */
   	gtk_widget_show(*item);
   	
}


void create_subtree(GtkWidget **subtree,  GtkWidget **item)
{
	*subtree = gtk_tree_new ();
	g_print ("root subtree is %p\n", *subtree);
    /* This is still necessary if you want these signals to be called
		for the subtree's children.  Note that selection_change will be 
		signalled for the root tree regardless. */
    g_signal_connect (G_OBJECT (*subtree), "select_child"  , G_CALLBACK (cb_select_child), *subtree);
    g_signal_connect (G_OBJECT (*subtree), "unselect_child", G_CALLBACK (cb_unselect_child), *subtree);
    /* This has absolutely no effect, because it is completely ignored 
		in subtrees */
    gtk_tree_set_selection_mode (GTK_TREE (*subtree), GTK_SELECTION_SINGLE);
    /* Neither does this, but for a rather different reason - the
       view_mode and view_line values of a tree are propagated to
       subtrees when they are mapped.  So, setting it later on would
       actually have a (somewhat unpredictable) effect */
    gtk_tree_set_view_mode (GTK_TREE (*subtree), GTK_TREE_VIEW_ITEM);
    /* Set this item's subtree - note that you cannot do this until
       AFTER the item has been added to its parent tree! */
    gtk_tree_item_set_subtree (GTK_TREE_ITEM (*item), *subtree);
}

void create_tree(GtkWidget **tree, GtkWidget **scrolled_win)
{
	
	/* Create the root tree */
	*tree = gtk_tree_new ();
	
	/* connect all GtkTree:: signals */
	g_signal_connect (G_OBJECT (*tree), "select_child"    ,G_CALLBACK (cb_select_child), *tree);
	g_signal_connect (G_OBJECT (*tree), "unselect_child"  ,G_CALLBACK (cb_unselect_child), *tree);
	g_signal_connect (G_OBJECT(*tree), "selection_changed",G_CALLBACK(cb_selection_changed), *tree);
	/* Add it to the scrolled window */
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (*scrolled_win), *tree);
	/* Set the selection mode */
	gtk_tree_set_selection_mode (GTK_TREE (*tree), GTK_SELECTION_MULTIPLE);

	gtk_widget_show (*tree); 
	g_print ("root tree is %p\n", *tree); 
}


void
printSpaces(int spaces)
{
    int i;
    char s[20] = "";

    for (i = 0; i < spaces; i++)
        //printf(" ");
        strcat(s," ");
}

void
printDataDirectory(char* doRef, IedConnection con, int spaces, IedClientError error , GtkWidget  **subtree4)
{

    LinkedList dataAttributes = IedConnection_getDataDirectory(con, &error, doRef);

    //LinkedList dataAttributes = IedConnection_getDataDirectoryByFC(con, &error, doRef, MX);

    if (dataAttributes != NULL)
	{
        LinkedList dataAttribute = LinkedList_getNext(dataAttributes);

        while (dataAttribute != NULL) 
        {	
			GtkWidget  *itemDA, *subtree5;
			gchar *label;
			 
            char* daName = (char*) dataAttribute->data;
            char texte[300];
				
            sprintf(texte,"DA: %s\n", (char*) dataAttribute->data);
            
            label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
            
			/* Create a subtree4 itemDA */
			create_item(subtree4, &itemDA, &label);
			
			/* Create a subtree5 of the itemDA*/
			create_subtree(&subtree5, &itemDA);

            dataAttribute = LinkedList_getNext(dataAttribute);

            char daRef[129];
            sprintf(daRef, "%s.%s", doRef, daName);
            printDataDirectory(daRef, con, spaces + 2, error, &subtree5);
        }

    }
}

void printServerDir(IedConnection con,    IedClientError error, LinkedList deviceList, GtkWidget *window, GtkWidget **subtree1)
{
	LinkedList device = LinkedList_getNext(deviceList);
	gchar *label;
	while (device != NULL) 
    {	
		GtkWidget *itemLD, *subtree2;
        
		char texte[300];
         
		sprintf(texte, "LD: %s\n", (char*) device->data);
         
		label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
			
		/* Create a subtree1 itemLD */
		create_item(subtree1, &itemLD, &label);

		/* Create a subtree2 of the itemLD*/
		create_subtree(&subtree2, &itemLD);

		LinkedList logicalNodes = IedConnection_getLogicalDeviceDirectory(con, &error, (char*) device->data);

		LinkedList logicalNode = LinkedList_getNext(logicalNodes);

		while (logicalNode != NULL) 
		{
			GtkWidget *itemLN, *subtree3;
            
			sprintf(texte, "LN: %s\n", (char*) logicalNode->data);
         
			label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
			
			/* Create a subtree2 itemLN */
			create_item(&subtree2, &itemLN, &label);

			/* Create a subtree3 of the itemLN*/
			create_subtree(&subtree3, &itemLN);
				
            char lnRef[129];

			sprintf(lnRef, "%s/%s", (char*) device->data, (char*) logicalNode->data);

			LinkedList dataObjects = IedConnection_getLogicalNodeDirectory(con, &error, lnRef, ACSI_CLASS_DATA_OBJECT);

			LinkedList dataObject = LinkedList_getNext(dataObjects);
			
			GtkWidget *itemD;

			while (dataObject != NULL) 
			{
				GtkWidget *subtree4;
				
				char* dataObjectName = (char*) dataObject->data;

				sprintf(texte, "DO: %s\n", dataObjectName);
         
				label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
			
				/* Create a subtree3 itemD */
				create_item(&subtree3, &itemD, &label);
				
				/* Create a subtree4 of the itemD*/
				create_subtree(&subtree4, &itemD);

                dataObject = LinkedList_getNext(dataObject);

                char doRef[129];

                sprintf(doRef, "%s/%s.%s", (char*) device->data, (char*) logicalNode->data, dataObjectName);

                printDataDirectory(doRef, con, 6, error, &subtree4);
                
			}

			LinkedList_destroy(dataObjects);

			LinkedList dataSets = IedConnection_getLogicalNodeDirectory(con, &error, lnRef, ACSI_CLASS_DATA_SET);

			LinkedList dataSet = LinkedList_getNext(dataSets);

			while (dataSet != NULL) 
            {
				GtkWidget  *subtree5;
					
				char* dataSetName = (char*) dataSet->data;
				bool isDeletable;
				char dataSetRef[129];
				sprintf(dataSetRef, "%s.%s", lnRef, dataSetName);

				LinkedList dataSetMembers = IedConnection_getDataSetDirectory(con, &error, dataSetRef, &isDeletable);

				if (isDeletable)
				{
               
					sprintf(texte,"Data set: %s (deletable)\n", dataSetName);
         
					label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
					
					/* Create a subtree3 itemD */
					create_item(&subtree3, &itemD, &label);

					/* Create a subtree5 of the itemD*/
					create_subtree(&subtree5, &itemD);
				}
				
				else
                       
				{
					sprintf(texte,"Data set: %s (not deletable)\n", dataSetName);
         
					label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
						
					/* Create a subtree3 itemD */
					create_item(&subtree3, &itemD, &label);

					/* Create a subtree5 of the itemD*/
					create_subtree(&subtree5, &itemD);
				}
				
                LinkedList dataSetMemberRef = LinkedList_getNext(dataSetMembers);

				while (dataSetMemberRef != NULL) 
				{
					GtkWidget  *itemDataSet;
					char* memberRef = (char*) dataSetMemberRef->data;

					sprintf(texte,"%s\n", memberRef);
         
					label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
						
					/* Create a subtree5 itemDataSet */
					create_item(&subtree5, &itemDataSet, &label);
					
					dataSetMemberRef = LinkedList_getNext(dataSetMemberRef);
				}

                dataSet = LinkedList_getNext(dataSet);
			}

			LinkedList_destroy(dataSets);

			LinkedList reports = IedConnection_getLogicalNodeDirectory(con, &error, lnRef, ACSI_CLASS_URCB);

			LinkedList report = LinkedList_getNext(reports);

			while (report != NULL) 
			{
				char* reportName = (char*) report->data;

				sprintf(texte,"RP: %s\n", reportName);
         	
				label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
				
				/* Create a subtree3 itemD */
				create_item(&subtree3, &itemD, &label);
                   
				report = LinkedList_getNext(report);
			}

			LinkedList_destroy(reports);

			reports = IedConnection_getLogicalNodeDirectory(con, &error, lnRef, ACSI_CLASS_BRCB);

			report = LinkedList_getNext(reports);

			while (report != NULL) 
			{
				char* reportName = (char*) report->data;
    
				sprintf(texte,"BR: %s\n", reportName);
         	
				label = g_locale_to_utf8(texte, -1, NULL, NULL, NULL);
				
				/* Create a subtree3 itemD */
				create_item(&subtree3, &itemD, &label);
				
				report = LinkedList_getNext(report);
                
			}

			LinkedList_destroy(reports);

			logicalNode = LinkedList_getNext(logicalNode);
		}

		LinkedList_destroy(logicalNodes);

		device = LinkedList_getNext(device);
	}
}

int
main(int argc, char** argv)
{
	GtkWidget *window, *scrolled_win, *tree;
	GtkWidget *subtree1, *item;
	static gchar *serverName = "Server 2";

    char* hostname;
    int tcpPort = 102;

    if (argc > 1)
        hostname = argv[1];
    else
        hostname = "localhost";

    if (argc > 2)
        tcpPort = atoi(argv[2]);

    IedClientError error;

    IedConnection con = IedConnection_create();

    IedConnection_connect(con, &error, hostname, tcpPort);

    if (error == IED_ERROR_OK) 
    {

        printf("Get logical device list...\n");
        LinkedList deviceList = IedConnection_getLogicalDeviceList(con, &error);

		if (error != IED_ERROR_OK)
		{
			printf("Failed to read device list (error code: %i)\n", error);
			goto cleanup_and_exit;
        }
        gtk_init (&argc, &argv);

		/* a generic toplevel window */
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		gtk_container_set_border_width (GTK_CONTAINER (window), 5);

		/* A generic scrolled window */
		scrolled_win = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
					GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
		gtk_widget_set_size_request (scrolled_win, 600, 500);
		gtk_container_add (GTK_CONTAINER (window), scrolled_win);
		gtk_widget_show (scrolled_win);
	
		/* Create the root tree */
		create_tree(&tree, &scrolled_win);
	
		/* Create a tree item name server */
		create_item(&tree, &item, &serverName);
	
		/* Create a subtree of the item*/
		create_subtree(&subtree1, &item);
		
		printServerDir(con,	error,	deviceList, window, &subtree1);
		
		/* Show the window and loop endlessly */
		gtk_widget_show (window);
		gtk_main();
	
        LinkedList_destroy(deviceList);
 
        IedConnection_close(con);
    }
    
    else 
    {
        printf("Connection failed!\n");
    }
    
cleanup_and_exit:

    IedConnection_destroy(con);
}


