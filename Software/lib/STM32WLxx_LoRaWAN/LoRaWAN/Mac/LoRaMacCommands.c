/*!
 * \file      LoRaMacCommands.c
 *
 * \brief     LoRa MAC commands
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */
#include <stddef.h>

#include "utilities.h"
#include "LoRaMacCommands.h"
#include "LoRaMacConfirmQueue.h"

/*!
 * Number of MAC Command slots
 */
#define NUM_OF_MAC_COMMANDS 15

/*!
 * Size of the CID field of MAC commands
 */
#define CID_FIELD_SIZE 1

/*!
 *  Mac Commands list structure
 */
typedef struct sMacCommandsList
{
    /*
     * First element of MAC command list.
     */
    MacCommand_t* First;
    /*
     * Last element of MAC command list.
     */
    MacCommand_t* Last;
} MacCommandsList_t;

/*!
 * LoRaMac Commands Context structure
 */
typedef struct sLoRaMacCommandsCtx
{
    /*
     * List of MAC command elements
     */
    MacCommandsList_t MacCommandList;
    /*
     * Buffer to store MAC command elements
     */
    MacCommand_t MacCommandSlots[NUM_OF_MAC_COMMANDS];
    /*
     * Size of all MAC commands serialized as buffer
     */
    size_t SerializedCmdsSize;
} LoRaMacCommandsCtx_t;

/*!
 * Callback function to notify the upper layer about context change
 */
static LoRaMacCommandsNvmEvent CommandsNvmCtxChanged;

/*!
 * Non-volatile module context.
 */
static LoRaMacCommandsCtx_t NvmCtx;

/* Memory management functions */

/*!
 * \brief Determines if a MAC command slot is free
 *
 * \param[IN]     slot           - Slot to check
 * \retval                       - Status of the operation
 */
static bool IsSlotFree( const MacCommand_t* slot )
{
    uint8_t* mem = ( uint8_t* )slot;

    for( uint16_t size = 0; size < sizeof( MacCommand_t ); size++ )
    {
        if( mem[size] != 0x00 )
        {
            return false;
        }
    }
    return true;
}

/*!
 * \brief Allocates a new MAC command memory slot
 *
 * \retval                       - Pointer to slot
 */
static MacCommand_t* MallocNewMacCommandSlot( void )
{
    uint8_t itr = 0;

    while( IsSlotFree( ( const MacCommand_t* )&NvmCtx.MacCommandSlots[itr] ) == false )
    {
        itr++;
        if( itr == NUM_OF_MAC_COMMANDS )
        {
            return NULL;
        }
    }

    return &NvmCtx.MacCommandSlots[itr];
}

/*!
 * \brief Free memory slot
 *
 * \param[IN]     slot           - Slot to free
 *
 * \retval                       - Status of the operation
 */
static bool FreeMacCommandSlot( MacCommand_t* slot )
{
    if( slot == NULL )
    {
        return false;
    }

    memset1( ( uint8_t* )slot, 0x00, sizeof( MacCommand_t ) );

    return true;
}

/* Linked list functions */

/*!
 * \brief Initialize list
 *
 * \param[IN]     list           - List that shall be initialized
 * \retval                       - Status of the operation
 */
static bool LinkedListInit( MacCommandsList_t* list )
{
    if( list == NULL )
    {
        return false;
    }

    list->First = NULL;
    list->Last = NULL;

    return true;
}

/*!
 * \brief Add an element to the list
 *
 * \param[IN]     list           - List where the element shall be added.
 * \param[IN]     element        - Element to add
 * \retval                       - Status of the operation
 */
static bool LinkedListAdd( MacCommandsList_t* list, MacCommand_t* element )
{
    if( ( list == NULL ) || ( element == NULL ) )
    {
        return false;
    }

    // Check if this is the first entry to enter the list.
    if( list->First == NULL )
    {
        list->First = element;
    }

    // Check if the last entry exists and update its next point.
    if( list->Last )
    {
        list->Last->Next = element;
    }

    // Update the next point of this entry.
    element->Next = NULL;

    // Update the last entry of the list.
    list->Last = element;

    return true;
}

/*!
 * \brief Return the previous element in the list.
 *
 * \param[IN]     list           - List
 * \param[IN]     element        - Element where the previous element shall be searched
 * \retval                       - Status of the operation
 */
static MacCommand_t* LinkedListGetPrevious( MacCommandsList_t* list, MacCommand_t* element )
{
    if( ( list == NULL ) || ( element == NULL ) )
    {
        return NULL;
    }

    MacCommand_t* curElement;

    // Start at the head of the list
    curElement = list->First;

    // When current element is the first of the list, there's no previous element so we can return NULL immediately.
    if( element != curElement )
    {
        // Loop through all elements until the end is reached or the next of current is the current element.
        while( ( curElement != NULL ) && ( curElement->Next != element ) )
        {
            curElement = curElement->Next;
        }
    }
    else
    {
        curElement = NULL;
    }

    return curElement;
}

/*!
 * \brief Remove an element from the list
 *
 * \param[IN]     list           - List where the element shall be removed from.
 * \param[IN]     element        - Element to remove
 * \retval                       - Status of the operation
 */
static bool LinkedListRemove( MacCommandsList_t* list, MacCommand_t* element )
{
    if( ( list == NULL ) || ( element == NULL ) )
    {
        return false;
    }

    MacCommand_t* PrevElement = LinkedListGetPrevious( list, element );

    if( list->First == element )
    {
        list->First = element->Next;
    }

    if( list->Last == element )
    {
        list->Last = PrevElement;
    }

    if( PrevElement != NULL )
    {
        PrevElement->Next = element->Next;
    }

    element->Next = NULL;

    return true;
}

/*
 * \brief Determines if a MAC command is sticky or not
 *
 * \param[IN]   cid                - MAC command identifier
 *
 * \retval                     - Status of the operation
 */
static bool IsSticky( uint8_t cid )
{
    switch( cid )
    {
        case MOTE_MAC_DL_CHANNEL_ANS:
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
        case MOTE_MAC_TX_PARAM_SETUP_ANS:
            return true;
        default:
            return false;
    }
}

/*
 * \brief Wrapper function for the NvmCtx
 */
static void NvmCtxCallback( void )
{
    if( CommandsNvmCtxChanged != NULL )
    {
        CommandsNvmCtxChanged( );
    }
}

LoRaMacCommandStatus_t LoRaMacCommandsInit( LoRaMacCommandsNvmEvent commandsNvmCtxChanged )
{
    // Initialize with default
    memset1( ( uint8_t* )&NvmCtx, 0, sizeof( NvmCtx ) );

    LinkedListInit( &NvmCtx.MacCommandList );

    // Assign callback
    CommandsNvmCtxChanged = commandsNvmCtxChanged;

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsRestoreNvmCtx( void* commandsNvmCtx )
{
    // Restore module context
    if( commandsNvmCtx != NULL )
    {
        memcpy1( ( uint8_t* )&NvmCtx, ( uint8_t* )commandsNvmCtx, sizeof( NvmCtx ) );
        return LORAMAC_COMMANDS_SUCCESS;
    }
    else
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }
}

void* LoRaMacCommandsGetNvmCtx( size_t* commandsNvmCtxSize )
{
    *commandsNvmCtxSize = sizeof( NvmCtx );
    return &NvmCtx;
}

LoRaMacCommandStatus_t LoRaMacCommandsAddCmd( uint8_t cid, uint8_t* payload, size_t payloadSize )
{
    if( payload == NULL )
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }
    MacCommand_t* newCmd;

    // Allocate a memory slot
    newCmd = MallocNewMacCommandSlot( );

    if( newCmd == 0 )
    {
        return LORAMAC_COMMANDS_ERROR_MEMORY;
    }

    // Add it to the list of Mac commands
    if( LinkedListAdd( &NvmCtx.MacCommandList, newCmd ) == false )
    {
        return LORAMAC_COMMANDS_ERROR;
    }

    // Set Values
    newCmd->CID = cid;
    newCmd->PayloadSize = payloadSize;
    memcpy1( ( uint8_t* )newCmd->Payload, payload, payloadSize );
    newCmd->IsSticky = IsSticky( cid );

    NvmCtx.SerializedCmdsSize += ( CID_FIELD_SIZE + payloadSize );

    NvmCtxCallback( );

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsRemoveCmd( MacCommand_t* macCmd )
{
    if( macCmd == NULL )
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }

    // Remove the Mac command element from MacCommandList
    if( LinkedListRemove( &NvmCtx.MacCommandList, macCmd ) == false )
    {
        return LORAMAC_COMMANDS_ERROR_CMD_NOT_FOUND;
    }

    NvmCtx.SerializedCmdsSize -= ( CID_FIELD_SIZE + macCmd->PayloadSize );

    // Free the MacCommand Slot
    if( FreeMacCommandSlot( macCmd ) == false )
    {
        return LORAMAC_COMMANDS_ERROR;
    }

    NvmCtxCallback( );

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsGetCmd( uint8_t cid, MacCommand_t** macCmd )
{
    MacCommand_t* curElement;

    // Start at the head of the list
    curElement = NvmCtx.MacCommandList.First;

    // Loop through all elements until we find the element with the given CID
    while( ( curElement != NULL ) && ( curElement->CID != cid ) )
    {
        curElement = curElement->Next;
    }

    // Update the pointer anyway
    *macCmd = curElement;

    // Handle error in case if we reached the end without finding it.
    if( curElement == NULL )
    {
        return LORAMAC_COMMANDS_ERROR_CMD_NOT_FOUND;
    }
    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsRemoveNoneStickyCmds( void )
{
    MacCommand_t* curElement;
    MacCommand_t* nexElement;

    // Start at the head of the list
    curElement = NvmCtx.MacCommandList.First;

    // Loop through all elements
    while( curElement != NULL )
    {
        if( curElement->IsSticky == false )
        {
            nexElement = curElement->Next;
            LoRaMacCommandsRemoveCmd( curElement );
            curElement = nexElement;
        }
        else
        {
            curElement = curElement->Next;
        }
    }

    NvmCtxCallback( );

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsRemoveStickyAnsCmds( void )
{
    MacCommand_t* curElement;
    MacCommand_t* nexElement;

    // Start at the head of the list
    curElement = NvmCtx.MacCommandList.First;

    // Loop through all elements
    while( curElement != NULL )
    {
        nexElement = curElement->Next;
        if( IsSticky( curElement->CID ) == true )
        {
            LoRaMacCommandsRemoveCmd( curElement );
        }
        curElement = nexElement;
    }

    NvmCtxCallback( );

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsGetSizeSerializedCmds( size_t* size )
{
    if( size == NULL )
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }
    *size = NvmCtx.SerializedCmdsSize;
    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsSerializeCmds( size_t availableSize, size_t* effectiveSize, uint8_t* buffer )
{
    MacCommand_t* curElement = NvmCtx.MacCommandList.First;
    MacCommand_t* nextElement;
    uint8_t itr = 0;

    if( ( buffer == NULL ) || ( effectiveSize == NULL ) )
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }

    // Loop through all elements which fits into the buffer
    while( curElement != NULL )
    {
        // If the next MAC command still fits into the buffer, add it.
        if( ( availableSize - itr ) >= ( CID_FIELD_SIZE + curElement->PayloadSize ) )
        {
            buffer[itr++] = curElement->CID;
            memcpy1( &buffer[itr], curElement->Payload, curElement->PayloadSize );
            itr += curElement->PayloadSize;
        }
        else
        {
            break;
        }
        curElement = curElement->Next;
    }

    // Remove all commands which do not fit into the buffer
    while( curElement != NULL )
    {
        // Store the next element before removing the current one
        nextElement = curElement->Next;
        LoRaMacCommandsRemoveCmd( curElement );
        curElement = nextElement;
    }

    // Fetch the effective size of the mac commands
    LoRaMacCommandsGetSizeSerializedCmds( effectiveSize );

    return LORAMAC_COMMANDS_SUCCESS;
}

LoRaMacCommandStatus_t LoRaMacCommandsStickyCmdsPending( bool* cmdsPending )
{
    if( cmdsPending == NULL )
    {
        return LORAMAC_COMMANDS_ERROR_NPE;
    }
    MacCommand_t* curElement;
    curElement = NvmCtx.MacCommandList.First;

    *cmdsPending = false;

    // Loop through all elements
    while( curElement != NULL )
    {
        if( curElement->IsSticky == true )
        {
            // Found one sticky MAC command
            *cmdsPending = true;
            return LORAMAC_COMMANDS_SUCCESS;
        }
        curElement = curElement->Next;
    }

    return LORAMAC_COMMANDS_SUCCESS;
}

uint8_t LoRaMacCommandsGetCmdSize( uint8_t cid )
{
    uint8_t cidSize = 0;

    // Decode Frame MAC commands
    switch( cid )
    {
        case SRV_MAC_LINK_CHECK_ANS:
        {
            // cid + Margin + GwCnt
            cidSize = 3;
            break;
        }
        case SRV_MAC_LINK_ADR_REQ:
        {
            // cid + DataRate_TXPower + ChMask (2) + Redundancy
            cidSize = 5;
            break;
        }
        case SRV_MAC_DUTY_CYCLE_REQ:
        {
            // cid + DutyCyclePL
            cidSize = 2;
            break;
        }
        case SRV_MAC_RX_PARAM_SETUP_REQ:
        {
            // cid + DLsettings + Frequency (3)
            cidSize = 5;
            break;
        }
        case SRV_MAC_DEV_STATUS_REQ:
        {
            // cid
            cidSize = 1;
            break;
        }
        case SRV_MAC_NEW_CHANNEL_REQ:
        {
            // cid + ChIndex + Frequency (3) + DrRange
            cidSize = 6;
            break;
        }
        case SRV_MAC_RX_TIMING_SETUP_REQ:
        {
            // cid + Settings
            cidSize = 2;
            break;
        }
        case SRV_MAC_TX_PARAM_SETUP_REQ:
        {
            // cid + EIRP_DwellTime
            cidSize = 2;
            break;
        }
        case SRV_MAC_DL_CHANNEL_REQ:
        {
            // cid + ChIndex + Frequency (3)
            cidSize = 5;
            break;
        }
        case SRV_MAC_DEVICE_TIME_ANS:
        {
            // cid + Seconds (4) + Fractional seconds (1)
            cidSize = 6;
            break;
        }
        case SRV_MAC_PING_SLOT_INFO_ANS:
        {
            // cid
            cidSize = 1;
            break;
        }
        case SRV_MAC_PING_SLOT_CHANNEL_REQ:
        {
            // cid + Frequency (3) + DR
            cidSize = 5;
            break;
        }
        case SRV_MAC_BEACON_TIMING_ANS:
        {
            // cid + TimingDelay (2) + Channel
            cidSize = 4;
            break;
        }
        case SRV_MAC_BEACON_FREQ_REQ:
        {
            // cid + Frequency (3)
            cidSize = 4;
            break;
        }
        default:
        {
            // Unknown command. ABORT MAC commands processing
            break;
        }
    }
    return cidSize;
}
