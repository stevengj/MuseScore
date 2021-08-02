/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_NOTATION_NOTATIONPARTS_H
#define MU_NOTATION_NOTATIONPARTS_H

#include "inotationparts.h"
#include "async/asyncable.h"
#include "inotationundostack.h"
#include "inotationinteraction.h"

namespace mu::notation {
class IGetScore;
class NotationParts : public INotationParts, public async::Asyncable
{
public:
    NotationParts(IGetScore* getScore, INotationInteractionPtr interaction, INotationUndoStackPtr undoStack);
    ~NotationParts() override;

    async::NotifyList<const Part*> partList() const override;
    async::NotifyList<Instrument> instrumentList(const ID& partId) const override;
    async::NotifyList<const Staff*> staffList(const ID& partId) const override;

    void setParts(const PartInstrumentList& parts) override;
    void setScoreOrder(const ScoreOrder& order) override;
    void setPartVisible(const ID& partId, bool visible) override;
    void setVoiceVisible(const ID& staffId, int voiceIndex, bool visible) override;
    void setStaffVisible(const ID& staffId, bool visible) override;
    void setPartName(const ID& partId, const QString& name) override;
    void setPartSharpFlat(const ID& partId, const SharpFlat& sharpFlat) override;
    void setPartTransposition(const ID& partId, const Interval& transpose) override;
    void setInstrumentName(const ID& instrumentId, const ID& fromPartId, const QString& name) override;
    void setInstrumentAbbreviature(const ID& instrumentId, const ID& fromPartId, const QString& abbreviature) override;
    void setStaffType(const ID& staffId, StaffType type) override;
    void setCutawayEnabled(const ID& staffId, bool enabled) override;
    void setSmallStaff(const ID& staffId, bool smallStaff) override;
    void setStaffConfig(const ID& staffId, const StaffConfig& config) override;

    void removeParts(const IDList& partsIds) override;
    void removeStaves(const IDList& stavesIds) override;

    void moveParts(const IDList& sourcePartsIds, const ID& destinationPartId, InsertMode mode = InsertMode::Before) override;
    void moveStaves(const IDList& sourceStavesIds, const ID& destinationStaffId, InsertMode mode = InsertMode::Before) override;

    void appendStaff(Staff* staff, const ID& destinationPartId) override;
    void cloneStaff(const ID& sourceStaffId, const ID& destinationStaffId) override;

    void replaceInstrument(const ID& instrumentId, const ID& fromPartId, const Instrument& newInstrument) override;
    void replaceDrumset(const ID& instrumentId, const ID& fromPartId, const Drumset& newDrumset) override;

    async::Notification partsChanged() const override;

protected:
    INotationUndoStackPtr undoStack() const;

    virtual void startEdit();
    virtual void apply();

private:
    struct InstrumentInfo
    {
        Ms::Fraction fraction = { -1, -1 };
        Ms::Instrument* instrument = nullptr;

        InstrumentInfo() = default;

        InstrumentInfo(const Ms::Fraction& fraction, Ms::Instrument* instrument)
            : fraction(fraction), instrument(instrument) {}

        bool isValid() const { return instrument != nullptr; }
    };

    struct InstrumentKey
    {
        ID partId;
        ID instrumentId;

        bool operator==(const InstrumentKey& key) const
        {
            return partId == key.partId && instrumentId == key.instrumentId;
        }

        friend uint qHash(const InstrumentKey& key)
        {
            return qHash(QString(key.partId + key.instrumentId));
        }
    };

    Ms::Score* score() const;
    Ms::MasterScore* masterScore() const;
    void updateScore();

    void updatePartTitles();

    void doMoveStaves(const std::vector<Staff*>& staves, int destinationStaffIndex, Part* destinationPart = nullptr);
    void doSetStaffVisible(Staff* staff, bool visible);
    void doSetStaffVoiceVisible(Staff* staff, int voiceIndex, bool visible);
    void doRemoveParts(const IDList& partsIds);
    void doSetPartName(Part* part, const QString& name);

    void insertStaff(Staff* staff, int destinationStaffIndex);

    Part* part(const ID& partId, const Ms::Score* score = nullptr) const;
    InstrumentInfo instrumentInfo(const ID& instrumentId, const Part* fromPart) const;
    InstrumentInfo instrumentInfo(const Staff* staff) const;

    Staff* staff(const ID& staffId) const;
    std::vector<const Staff*> staves(const Part* part) const;
    std::vector<Staff*> staves(const IDList& stavesIds) const;

    std::vector<Part*> availableParts(const Ms::Score* score) const;
    std::vector<Part*> scoreParts(const Ms::Score* score) const;
    std::vector<Part*> excerptParts(const Ms::Score* score) const;

    void appendPart(Part* part);
    int resolvePartIndex(Part* part) const;

    void appendStaves(Part* part, const Instrument& instrument);

    void removeMissingParts(const PartInstrumentList& parts);
    void appendNewParts(const PartInstrumentList& parts);
    void updateSoloist(const PartInstrumentList& parts);
    void sortParts(const PartInstrumentList& parts, const Ms::Score* score, const QList<Ms::Staff*>& originalStaves);

    int resolveInstrumentNumber(const Instruments& newInstruments, const Instrument& currentInstrument) const;

    int lastStaffIndex() const;

    void initStaff(Staff* staff, const Instrument& instrument, const Ms::StaffType* staffType, int cleffIndex);

    void notifyAboutPartsChanged() const;
    void notifyAboutPartChanged(const ID& partId) const;
    void notifyAboutStaffChanged(const ID& staffId) const;

    async::ChangedNotifier<Instrument>* partNotifier(const ID& partId) const;
    async::ChangedNotifier<const Staff*>* instrumentNotifier(const ID& instrumentId, const ID& fromPartId) const;

    QString formatPartName(const Part* part) const;

    QMap<Ms::Fraction, Ms::Instrument*> instruments(const Part* fromPart) const;

    IGetScore* m_getScore = nullptr;
    INotationUndoStackPtr m_undoStack;
    async::Notification m_partsChanged;

    mutable async::ChangedNotifier<const Part*>* m_partsNotifier = nullptr;
    mutable std::map<ID, async::ChangedNotifier<Instrument>*> m_partsNotifiersMap;
    mutable QHash<InstrumentKey, async::ChangedNotifier<const Staff*>*> m_instrumentsNotifiersHash;
};
}

namespace Ms {
inline uint qHash(const Ms::Fraction& fraction)
{
    return qHash(QString::number(fraction.ticks()));
}
}

#endif // MU_NOTATION_NOTATIONPARTS_H
