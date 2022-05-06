/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 ******************************************************************************/
package org.epics.pva.server;

import static org.epics.pva.PVASettings.logger;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentHashMap.KeySetView;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.Level;

import org.epics.pva.data.PVAString;
import org.epics.pva.data.PVAStructure;

/** A served PV with data
 *
 *  <p>When updating the data, subscribed clients
 *  receive the changed data elements.
 *
 *  @author Kay Kasemir
 */
@SuppressWarnings("nls")
public class ServerPV
{
    /** Value used when accessing an RPC PV as a data PV */
    private static final PVAStructure RPC_SERVICE_VALUE = new PVAStructure("", "", new PVAString("value", "This is an RPC server"));

    /** Value returned from RPC call to data PV */
    private static final PVAStructure NO_SERVICE_VALUE = new PVAStructure("", "", new PVAString("value", "This is no RPC server"));

    /** Service implementation when accessing a data PV as an RPC service */
    private static final RPCService DEFAULT_RPC_SERVICE = request -> NO_SERVICE_VALUE;

    private static final AtomicInteger IDs = new AtomicInteger();

    private final String name;
    private final int sid;

    /** Current value
     *
     *  <p>Updates need to SYNC on data
     */
    private final PVAStructure data;

    private final RPCService rpc;

    /** All the 'monitor' subscriptions to this PV */
    private final KeySetView<MonitorSubscription, Boolean> subscriptions = ConcurrentHashMap.newKeySet();

    /** Create PV for serving data
     *  @param name PV name
     *  @param data Initial value
     */
    ServerPV(final String name, final PVAStructure data)
    {
        this.name = name;
        this.sid = IDs.incrementAndGet();
        this.data = data.cloneData();
        rpc = DEFAULT_RPC_SERVICE;
    }

    /** Create PV for handling RPC calls
     *  @param name PV name
     *  @param rpc {@link RPCService}
     */
    ServerPV(final String name, final RPCService rpc)
    {
        this.name = name;
        this.sid = IDs.incrementAndGet();
        this.data = RPC_SERVICE_VALUE;
        this.rpc = rpc;
    }

    public int getSID()
    {
        return sid;
    }

    /** @param subscription Subscription that needs to receive value updates */
    void register(final MonitorSubscription subscription)
    {
        logger.log(Level.FINER, () -> "Add " + subscription);
        subscriptions.add(subscription);
    }

    /** Forget monitor subscriptions
     *  @param tcp TCP connection for which to forget monitors
     *  @param req Specific monitor request or -1 to forget subscriptions for that connection
     */
    void unregister(final ServerTCPHandler tcp, final int req)
    {
        for (MonitorSubscription subscription : subscriptions)
            if (subscription.isFor(tcp, req))
            {
                logger.log(Level.FINER, () -> "Remove " + subscription);
                subscriptions.remove(subscription);
                break;
            }
    }

    /** Update the PV's data
     *
     *  <p>The new data is used to update the current
     *  value of the PV.
     *  Its type must match the initial value used when
     *  creating the PV on the server.
     *
     *  @param new_data New data to serve
     *  @throws Exception on error
     */
    public void update(final PVAStructure new_data) throws Exception
    {
        // Update data
        synchronized (data)
        {
            data.update(new_data);
        }
        // Update subscriptions
        for (MonitorSubscription subscription : subscriptions)
            subscription.update(new_data);
    }

    /** Get current value
     *  @return PV's current data
     */
    PVAStructure getData()
    {
        synchronized (data)
        {
            return data.cloneData();
        }
    }

    /** Incoke RPC service
     *  @param parameters RPC parameters
     *  @return RPC result
     *  @throws Exception on error, for example invalid parameters
     */
    PVAStructure call(final PVAStructure parameters) throws Exception
    {
        return rpc.call(parameters);
    }

    @Override
    public String toString()
    {
        return name + " [SID " + sid + "]";
    }
}
