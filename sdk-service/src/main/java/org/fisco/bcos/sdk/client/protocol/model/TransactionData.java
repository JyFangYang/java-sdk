// **********************************************************************
// This file was generated by a TARS parser!
// TARS version 1.7.2.
// **********************************************************************

package org.fisco.bcos.sdk.client.protocol.model;

import com.qq.tars.protocol.tars.TarsInputStream;
import com.qq.tars.protocol.tars.TarsOutputStream;
import com.qq.tars.protocol.tars.annotation.TarsStruct;
import com.qq.tars.protocol.tars.annotation.TarsStructProperty;
import com.qq.tars.protocol.util.TarsUtil;

@TarsStruct
public class TransactionData {

    @TarsStructProperty(order = 1, isRequire = true)
    public int version = 0;

    @TarsStructProperty(order = 2, isRequire = true)
    public String chainID = "";

    @TarsStructProperty(order = 3, isRequire = true)
    public String groupID = "";

    @TarsStructProperty(order = 4, isRequire = true)
    public long blockLimit = 0L;

    @TarsStructProperty(order = 5, isRequire = true)
    public String nonce = "";

    @TarsStructProperty(order = 6, isRequire = false)
    public String to = "";

    @TarsStructProperty(order = 7, isRequire = true)
    public byte[] input = null;

    public int getVersion() {
        return this.version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public String getChainID() {
        return this.chainID;
    }

    public void setChainID(String chainID) {
        this.chainID = chainID;
    }

    public String getGroupID() {
        return this.groupID;
    }

    public void setGroupID(String groupID) {
        this.groupID = groupID;
    }

    public long getBlockLimit() {
        return this.blockLimit;
    }

    public void setBlockLimit(long blockLimit) {
        this.blockLimit = blockLimit;
    }

    public String getNonce() {
        return this.nonce;
    }

    public void setNonce(String nonce) {
        this.nonce = nonce;
    }

    public String getTo() {
        return this.to;
    }

    public void setTo(String to) {
        this.to = to;
    }

    public byte[] getInput() {
        return this.input;
    }

    public void setInput(byte[] input) {
        this.input = input;
    }

    public TransactionData() {}

    public TransactionData(
            int version,
            String chainID,
            String groupID,
            long blockLimit,
            String nonce,
            String to,
            byte[] input) {
        this.version = version;
        this.chainID = chainID;
        this.groupID = groupID;
        this.blockLimit = blockLimit;
        this.nonce = nonce;
        this.to = to;
        this.input = input;
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + TarsUtil.hashCode(this.version);
        result = prime * result + TarsUtil.hashCode(this.chainID);
        result = prime * result + TarsUtil.hashCode(this.groupID);
        result = prime * result + TarsUtil.hashCode(this.blockLimit);
        result = prime * result + TarsUtil.hashCode(this.nonce);
        result = prime * result + TarsUtil.hashCode(this.to);
        result = prime * result + TarsUtil.hashCode(this.input);
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (!(obj instanceof TransactionData)) {
            return false;
        }
        TransactionData other = (TransactionData) obj;
        return (TarsUtil.equals(this.version, other.version)
                && TarsUtil.equals(this.chainID, other.chainID)
                && TarsUtil.equals(this.groupID, other.groupID)
                && TarsUtil.equals(this.blockLimit, other.blockLimit)
                && TarsUtil.equals(this.nonce, other.nonce)
                && TarsUtil.equals(this.to, other.to)
                && TarsUtil.equals(this.input, other.input));
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("TransactionData(");
        sb.append("version:");
        sb.append(this.version);
        sb.append(", ");
        sb.append("chainID:");
        if (this.chainID == null) {
            sb.append("null");
        } else {
            sb.append(this.chainID);
        }
        sb.append(", ");
        sb.append("groupID:");
        if (this.groupID == null) {
            sb.append("null");
        } else {
            sb.append(this.groupID);
        }
        sb.append(", ");
        sb.append("blockLimit:");
        sb.append(this.blockLimit);
        sb.append(", ");
        sb.append("nonce:");
        if (this.nonce == null) {
            sb.append("null");
        } else {
            sb.append(this.nonce);
        }
        sb.append(", ");
        sb.append("to:");
        if (this.to == null) {
            sb.append("null");
        } else {
            sb.append(this.to);
        }
        sb.append(", ");
        sb.append("input:");
        if (this.input == null) {
            sb.append("null");
        } else {
            sb.append(this.input);
        }
        sb.append(")");
        return sb.toString();
    }

    public void writeTo(TarsOutputStream _os) {
        _os.write(this.version, 1);
        _os.write(this.chainID, 2);
        _os.write(this.groupID, 3);
        _os.write(this.blockLimit, 4);
        _os.write(this.nonce, 5);
        if (null != this.to) {
            _os.write(this.to, 6);
        }
        _os.write(this.input, 7);
    }

    static byte[] cache_input;

    static {
        cache_input = new byte[1];
        byte var_1 = (byte) 0;
        cache_input[0] = var_1;
    }

    public void readFrom(TarsInputStream _is) {
        this.version = _is.read(this.version, 1, true);
        this.chainID = _is.readString(2, true);
        this.groupID = _is.readString(3, true);
        this.blockLimit = _is.read(this.blockLimit, 4, true);
        this.nonce = _is.readString(5, true);
        this.to = _is.readString(6, false);
        this.input = (byte[]) _is.read(cache_input, 7, true);
    }
}
